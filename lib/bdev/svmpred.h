#ifndef SVMPRED_H
#define SVMPRED_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>

struct model;

// 声明 load_model 函数
struct model *load_model(const char *model_file_name) ;

// 声明 get_nr_feature 函数
int get_nr_feature(const struct model *model_) ;

// 声明 get_nr_class 函数
int get_nr_class(const struct model *model_) ;

// 声明 get_labels 函数
void get_labels(const struct model *model_, int* label) ;

// 声明 check_probability_model 函数
int check_probability_model(const struct model *model_) ;

// 声明 check_regression_model 函数
int check_regression_model(const struct model *model_) ;

// 声明 check_oneclass_model 函数
int check_oneclass_model(const struct model *model_) ;

// 声明 predict_tail_values 函数
double predict_tail_values(const struct model *model_, const double *x, double *dec_values) ;

// 声明 predict_tail 函数
double predict_tail(const struct model *model_, const double *x) ;

/* predict tail latency */
enum { L2R_LR, L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC, L2R_L1LOSS_SVC_DUAL, MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR, L2R_LR_DUAL, L2R_L2LOSS_SVR = 11, L2R_L2LOSS_SVR_DUAL, L2R_L1LOSS_SVR_DUAL, ONECLASS_SVM = 21 }; /* solver_type */
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
struct parameter
{
	int solver_type;

	/* these are for training only */
	double eps;             /* stopping tolerance */
	double C;
	int nr_weight;
	int *weight_label;
	double* weight;
	double p;
	double nu;
	double *init_sol;
	int regularize_bias;
};

struct model
{
	struct parameter param;
	int nr_class;           /* number of classes */
	int nr_feature;
	double *w;
	int *label;             /* label of each class */
	double bias;
	double rho;             /* one-class SVM only */
};

#define FSCANF(_stream, _format, _var)do\
{\
	if (fscanf(_stream, _format, _var) != 1)\
	{\
		fprintf(stderr, "ERROR: fscanf failed to read the model\n");\
		EXIT_LOAD_MODEL()\
	}\
}while(0)
// EXIT_LOAD_MODEL should NOT end with a semicolon.

#define EXIT_LOAD_MODEL()\
{\
	setlocale(LC_ALL, old_locale);\
	free(model_->label);\
	free(model_);\
	free(old_locale);\
	return NULL;\
}

static const char *solver_type_table[]=
{
	"L2R_LR", "L2R_L2LOSS_SVC_DUAL", "L2R_L2LOSS_SVC", "L2R_L1LOSS_SVC_DUAL", "MCSVM_CS",
	"L1R_L2LOSS_SVC", "L1R_LR", "L2R_LR_DUAL",
	"", "", "",
	"L2R_L2LOSS_SVR", "L2R_L2LOSS_SVR_DUAL", "L2R_L1LOSS_SVR_DUAL",
	"", "", "", "", "", "", "",
	"ONECLASS_SVM", NULL
};
struct model *load_model(const char *model_file_name)
{
	FILE *fp = fopen(model_file_name,"r");
	if(fp==NULL) return NULL;

	int i;
	int nr_feature;
	int n;
	int nr_class;
	double bias;
	double rho;
	struct model *model_ = Malloc(struct model,1);
	struct parameter param = model_->param;
	// parameters for training only won't be assigned, but arrays are assigned as NULL for safety
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	param.init_sol = NULL;

	model_->label = NULL;

	char *old_locale = setlocale(LC_ALL, NULL);
	if (old_locale)
	{
		old_locale = strdup(old_locale);
	}
	setlocale(LC_ALL, "C");

	char cmd[81];
	while(1)
	{
		FSCANF(fp,"%80s",cmd);
		if(strcmp(cmd,"solver_type")==0)
		{
			FSCANF(fp,"%80s",cmd);
			int i;
			for(i=0;solver_type_table[i];i++)
			{
				if(strcmp(solver_type_table[i],cmd)==0)
				{
					param.solver_type=i;
					break;
				}
			}
			if(solver_type_table[i] == NULL)
			{
				fprintf(stderr,"unknown solver type.\n");
				EXIT_LOAD_MODEL()
			}
		}
		else if(strcmp(cmd,"nr_class")==0)
		{
			FSCANF(fp,"%d",&nr_class);
			model_->nr_class=nr_class;
		}
		else if(strcmp(cmd,"nr_feature")==0)
		{
			FSCANF(fp,"%d",&nr_feature);
			model_->nr_feature=nr_feature;
		}
		else if(strcmp(cmd,"bias")==0)
		{
			FSCANF(fp,"%lf",&bias);
			model_->bias=bias;
		}
		else if(strcmp(cmd,"rho")==0)
		{
			FSCANF(fp,"%lf",&rho);
			model_->rho=rho;
		}
		else if(strcmp(cmd,"w")==0)
		{
			break;
		}
		else if(strcmp(cmd,"label")==0)
		{
			int nr_class = model_->nr_class;
			model_->label = Malloc(int,nr_class);
			for(int i=0;i<nr_class;i++)
				FSCANF(fp,"%d",&model_->label[i]);
		}
		else
		{
			fprintf(stderr,"unknown text in model file: [%s]\n",cmd);
			EXIT_LOAD_MODEL()
		}
	}

	nr_feature=model_->nr_feature;
	if(model_->bias>=0)
		n=nr_feature+1;
	else
		n=nr_feature;
	int w_size = n;
	int nr_w;
	if(nr_class==2 && param.solver_type != MCSVM_CS)
		nr_w = 1;
	else
		nr_w = nr_class;

	model_->w=Malloc(double, w_size*nr_w);
	for(i=0; i<w_size; i++)
	{
		int j;
		for(j=0; j<nr_w; j++)
			FSCANF(fp, "%lf ", &model_->w[i*nr_w+j]);
	}

	setlocale(LC_ALL, old_locale);
	free(old_locale);

	if (ferror(fp) != 0 || fclose(fp) != 0) return NULL;

	return model_;
}

int get_nr_feature(const struct model *model_)
{
	return model_->nr_feature;
}

int get_nr_class(const struct model *model_)
{
	return model_->nr_class;
}

void get_labels(const struct model *model_, int* label)
{
	if (model_->label != NULL)
		for(int i=0;i<model_->nr_class;i++)
			label[i] = model_->label[i];
}

int check_probability_model(const struct model *model_)
{
	return (model_->param.solver_type==L2R_LR ||
			model_->param.solver_type==L2R_LR_DUAL ||
			model_->param.solver_type==L1R_LR);
}

int check_regression_model(const struct model *model_)
{
	return (model_->param.solver_type==L2R_L2LOSS_SVR ||
			model_->param.solver_type==L2R_L1LOSS_SVR_DUAL ||
			model_->param.solver_type==L2R_L2LOSS_SVR_DUAL);
}

int check_oneclass_model(const struct model *model_)
{
	return model_->param.solver_type == ONECLASS_SVM;
}

double predict_tail_values(const struct model *model_, const double *x, double *dec_values)
{
	int n; // 特征数量（包含bias）
	if(model_->bias>=0)
		n=model_->nr_feature+1;
	else
		n=model_->nr_feature;
	double *w=model_->w;
	int nr_class=model_->nr_class;
	int i;
	int nr_w; // number of weights per class
	if(nr_class==2 && model_->param.solver_type != MCSVM_CS)
		nr_w = 1;
	else
		nr_w = nr_class;

	const double *lx=x;
	for(i=0;i<nr_w;i++)
		dec_values[i] = 0;
	for(int j = 0; j < n; j++)
	{
		for(i=0;i<nr_w;i++) dec_values[i] += w[j*nr_w+i]*lx[j]; // 用各个部分的特征乘以权重计算输出值
	}

	if(check_oneclass_model(model_))
		dec_values[0] -= model_->rho;

	if(nr_class==2)
	{
		if(check_regression_model(model_))
			return dec_values[0];
		else if(check_oneclass_model(model_))
			return (dec_values[0]>0)?1:-1;
		else
			return (dec_values[0]>0)?model_->label[0]:model_->label[1];
	}
	else
	{
		int dec_max_idx = 0;
		for(i=1;i<nr_class;i++)
		{
			if(dec_values[i] > dec_values[dec_max_idx])
				dec_max_idx = i;
		}
		return model_->label[dec_max_idx];
	}
}

double predict_tail(const struct model *model_, const double *x)
{
	double *dec_values = Malloc(double, model_->nr_class);
	double label=predict_tail_values(model_, x, dec_values);
	free(dec_values);
	return label;
}

#endif /* SVMPRED_H */