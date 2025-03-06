#include <stdio.h>
#include "../lib/bdev/svmpred.h" // 添加svm模块
#include "time.h"
#include <unistd.h>
#define HIST_LEN 4
#define FEATURE_LEN 2
#define HIST_FEATURE_LEN  (HIST_LEN * FEATURE_LEN) // m * n 代表m个CIO的n个特征，从旧到新排列
double submit_ring[HIST_FEATURE_LEN] = {0};

int main() {
    char model_path[200];
    struct model *model_;
	snprintf(model_path, sizeof(model_path), "/home/ssd/tmp/spdk/model_path/liblinear_train_nvme0n1_128k_64q_randwrite_1300M.model");
	model_ = load_model(model_path);
    clock_t start, end;
    double cpu_time_used;
    double per_io_used;
    long iterations_per_second = 1500000;
    int cnt = 1;
    
    while (1) {
        start = clock();
        for (long i = 0; i < iterations_per_second; i++){
            memmove(submit_ring, submit_ring + FEATURE_LEN, (HIST_FEATURE_LEN - FEATURE_LEN) * sizeof(double));
            submit_ring[HIST_FEATURE_LEN - FEATURE_LEN + 0] = 1.0;
            submit_ring[HIST_FEATURE_LEN - FEATURE_LEN + 1] = 2.0;
            
            // 调用model_predict函数并获取结果
            double predict_label = predict_tail(model_, submit_ring);
            if (predict_label > 0.5) {
                submit_ring[HIST_FEATURE_LEN - FEATURE_LEN + 0] = 1.0;
            } else {
                submit_ring[HIST_FEATURE_LEN - FEATURE_LEN + 1] = 2.0;
            }
            cnt++;
        }
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        per_io_used = cpu_time_used / iterations_per_second*1000000;
        printf("cnt = %d, cpu_time_used = %.6f, per_io_used = %.3f\n", cnt, cpu_time_used, per_io_used);
        if (cpu_time_used < 1.0) {
            usleep((__useconds_t)((1.0 - cpu_time_used) * 1e6));
        }
    }
    
    return 0;
}
