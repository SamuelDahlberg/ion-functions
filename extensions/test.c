#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include "stuck.h"
#include "spike.h"
#include "utils.h"
#include "polycals.h"
#include "time_utils.h"
#include "gradient.h"
#include "wmm.h"

void arange(double *arr, size_t len);
signed char all(signed char *, size_t);
void print_array(signed char *, size_t);
void print_double_array(double *, size_t);
void print_array_size_t(size_t *array, size_t len);
char test_stuck(void);
char test_stuck_zero_num(void);
char test_stuck_neg_num(void);
char test_spike_simple(void);
char test_spike_l(void);
char test_spike_long(void);
char test_polyval(void);
char test_gradient(void);
char test_gradient2(void);
char test_gradient3(void);
char test_gradient4(void);
char test_gradient5(void);
char test_gradient6(void);
char test_time_month(void);
char test_time_vector(void);
char test_time_vector_split(void);
char test_mag_decl(void);
char test_velocity_corr(void);
char test_search_sorted(void);
char test_polycal(void);
void test(char (*func)(void));
char check_expected(double *out, double *expected, size_t len, double atol, double rtol);

extern bool nearly_equal(double, double, double);

static const char *message=NULL;

int main(int argc, char *argv[])
{
    test(&test_stuck);
    test(&test_stuck_zero_num);
    test(&test_stuck_neg_num);
    test(&test_spike_simple);
    test(&test_spike_l);
    test(&test_spike_long);
    test(&test_polyval);
    test(&test_gradient);
    test(&test_gradient2);
    test(&test_gradient3);
    test(&test_gradient4);
    test(&test_gradient5);
    test(&test_gradient6);
    test(&test_time_month);
    test(&test_time_vector);
    test(&test_time_vector_split);
    test(&test_mag_decl);
    test(&test_velocity_corr);
    test(&test_search_sorted);
    test(&test_polycal);
    return 0;
}


void test(char (*func)(void))
{
    message=NULL;
    if(func())
        printf("ok\n");
    else {
        printf("FAIL\n");
        if(message)
            printf("%s\n", message);
    }
}

char test_velocity_corr()
{
    velocity_profile in, out;
    double uu[10], vv[10], lat[10], lon[10], z[10];
    double uu_cor[10], vv_cor[10];
    int64_t timestamp[10];
    WMM_Model *wmm_model;
    printf("test_velocity_corr... ");
    if(wmm_initialize("ion_functions/data/WMM2010.COF", &wmm_model)) {
        message = "Error initializing models";
        printf("\n%s\n", wmm_errmsg);
        return false;
    }
    in.len = 10;
    in.uu = uu;
    in.vv = vv;
    in.lat = lat;
    in.lon = lon;
    in.z = z;
    in.timestamp = timestamp;
    out.len = 10;
    out.uu = uu_cor;
    out.vv = vv_cor;

    for(int i=0;i<10;i++)
    {
        uu[i] = -3.2;
        vv[i] = 18.2;
        lat[i] = 14.6846;
        lon[i] = -51.044;
        z[i] = -6./1000.;
        timestamp[i] = 3319563600 - 2208988800;
        uu_cor[i] = vv_cor[i] = 0;
    }
    if(wmm_velocity_correction(&in, wmm_model, &out) != 10) {
        message = "Incomplete processing";
        printf("\n");
        return false;
    }
    for(int i=0;i<10;i++) {
        if(fabs(uu_cor[i] - (-8.5136)) > 0.0001) {
            message = "uu_cor is incorrect";
            printf("\n%f != -8.5136\n", uu_cor[i]);
            return false;
        }
        if(fabs(vv_cor[i] - (16.4012)) > 0.0001) {
            message = "vv_cor is incorrect";
            printf("\n%f != 16.4012\n", vv_cor[i]);
            return false;
        }
    }
    if(wmm_free(wmm_model)) {
        message = "Error freeing models";
        printf("\n%s\n", wmm_errmsg);
        return false;
    }

    return true;
}

char test_mag_decl()
{
    double lat = 40.0;
    double lon = -120.0;
    double z = 0.0;
    int year = 1900;
    int mon = 1;
    int day = 1;
    double declination;
    WMM_Model *model;
    char filename[] = "ion_functions/data/WMM2010.COF";

    printf("test_mag_decl... ");
    if(wmm_initialize(filename, &model)) {
        message = "Error initializing models";
        printf("\n%s\n", wmm_errmsg);
        return false;
    }

    declination = wmm_declination(model, lat, lon, z, year, mon, day);
    if(fabs(declination - 26.184622) > 0.00001) {
        message = "Expected doesn't match received";
        printf("\n%f != 26.184622\n", declination);
        return false;
    }

    if(wmm_free(model)) {
        message = "Error freeing models";
        printf("\n%s\n", wmm_errmsg);
        return false;
    }
    return true;
}

char test_time_vector()
{
    double dat[] = {3580142023.566965, 3580142024.566965, 3580142025.566965, 3580142026.566965, 3580142027.566965};
    short int expected[] = {5, 5, 5, 5, 5};
    short int results[5];

    printf("test_time_vector... ");

    ntp_month_vector(results, dat, 5);
    for(int i=0;i<5;i++) {
        if(!(expected[i] == results[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            return false;
        }
    }
    return true;
}

char test_time_vector_split()
{
    double dat[] = {3565987200, 3568665600, 3571084800, 3573763200, 3576355200}; 
    short int expected[] = {0,1,2,3,4};
    short int results[5];

    printf("test_time_vector_split... ");

    ntp_month_vector(results, dat, 5);
    for(int i=0;i<5;i++) {
        if(!(expected[i] == results[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            return false;
        }
    }
    return true;
}

char test_time_month()
{
    double tval = 3580142023.566965;
    short int expected = 5;
    short int received;
    printf("test_time_month... ");
    received = ntp_month(tval);
    if(expected != received)
        return false;
    return true;
}

char test_gradient()
{
    double dat[] = {3., 5., 98., 99., 4.};
    double x[] = {1., 2., 3., 4., 5. };
    double grad_min = -50;
    double grad_max = 50;
    double mindx = 0;
    double startdat = 0;
    double toldat = 5;
    size_t len = 5;
    signed char expected[] = {1, 1, 0, 0, 1};
    signed char out[] = {1, 1, 1, 1, 1};
    printf("test_gradient... ");
    gradient(out, dat, x, len, grad_min, grad_max, mindx, startdat, toldat, -99);
    for(int i=0;i<len;i++) {
        if(!(expected[i]==out[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            print_double_array(dat,5);
            print_array(expected,5);
            print_array(out,5);
            return false;
        }
    }
    return true;
}

char test_gradient2()
{
    double dat[] = {3., 5., 98., 99., 4.};
    double x[] = {1., 2., 3., 4., 5. };
    double grad_min = -50;
    double grad_max = 50;
    double mindx = 0;
    double startdat = 100;
    double toldat = 5;
    size_t len = 5;
    signed char expected[] = {0, 0, 1, 1, 0};
    signed char out[] = {1, 1, 1, 1, 1};
    printf("test_gradient2... ");
    gradient(out, dat, x, len, grad_min, grad_max, mindx, startdat, toldat, -99);
    for(int i=0;i<len;i++) {
        if(!(expected[i]==out[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            print_double_array(dat,5);
            print_array(expected,5);
            print_array(out,5);
            return false;
        }
    }
    return true;
}

char test_gradient3()
{
    double dat[] = {3., 5., 98., 99., 4.};
    double x[] = {1., 2., 3., 3.1, 4. };
    double grad_min = -50;
    double grad_max = 50;
    double mindx = 0.2;
    double startdat = 0;
    double toldat = 5;
    size_t len = 5;
    signed char expected[] = {1, 1, 0, -99, 1};
    signed char out[] = {1, 1, 1, 1, 1};
    printf("test_gradient3... ");
    gradient(out, dat, x, len, grad_min, grad_max, mindx, startdat, toldat, -99);
    for(int i=0;i<len;i++) {
        if(!(expected[i]==out[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            print_double_array(dat,5);
            print_array(expected,5);
            print_array(out,5);
            return false;
        }
    }
    return true;
}

char test_gradient4()
{
    double dat[] = {3., 5., 98., 99., 4.};
    double x[] = {1., 2., 2.1, 2.4, 4. };
    double grad_min = -50;
    double grad_max = 50;
    double mindx = 0.5;
    double startdat = 0;
    double toldat = 5;
    size_t len = 5;
    signed char expected[] = {1, 1, -99, -99, 1};
    signed char out[] = {1, 1, 1, 1, 1};
    printf("test_gradient4... ");
    gradient(out, dat, x, len, grad_min, grad_max, mindx, startdat, toldat, -99);
    for(int i=0;i<len;i++) {
        if(!(expected[i]==out[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            print_double_array(dat,5);
            print_array(expected,5);
            print_array(out,5);
            return false;
        }
    }
    return true;
}

char test_gradient5()
{
    double dat[] = {3., 90., 98., 99., 4.};
    double x[] = {1., 2., 2.1, 2.4, 4. };
    double grad_min = -50;
    double grad_max = 50;
    double mindx = 0.5;
    double startdat = 0;
    double toldat = 5;
    size_t len = 5;
    signed char expected[] = {1, 0, -99, -99, 1};
    signed char out[] = {1, 1, 1, 1, 1};
    printf("test_gradient5... ");
    gradient(out, dat, x, len, grad_min, grad_max, mindx, startdat, toldat, -99);
    for(int i=0;i<len;i++) {
        if(!(expected[i]==out[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            print_double_array(dat,5);
            print_array(expected,5);
            print_array(out,5);
            return false;
        }
    }
    return true;
}

char test_gradient6() 
{
    double x[]   = {0. , 1. , 2. , 3. , 4. , 5. , 6. , 7. , 8. , 9.};
    double dat[] = {2. , 2. , 2. , 2. , 2. , 2. , 2. , 2. , 2. , 2.};
    double grad_min = -10.0;
    double grad_max = 10.0;
    double mindx = 2;
    double startdat = 0;
    double toldat = 5;
    size_t len = 10;
    signed char out[10];
    signed char expected[] = {1, -99, -99, 1, -99, -99, 1, -99, -99, 1};
    memset(out, 1, 10);
    printf("test_gradient6... ");
    gradient(out, dat, x, len, grad_min, grad_max, mindx, startdat, toldat, -99);
    for(int i=0;i<len;i++) {
        if(!(expected[i] == out[i])) {
            message = "Expected doesn't match received";
            printf("\n");
            print_double_array(dat, 10);
            print_array(expected, 5);
            print_array(out, 5);
            return false;
        }
    }
    return true;
}

char test_polyval()
{
    double p[] = {1., 2., -3.};
    double inputs[] = {0., 1., 2., 3.};
    double expected[] = {-3., 0., 5., 12.};
    double e = 0.00001;
    int i=0;
    printf("test_polyval... ");

    for(i=0;i<4;i++)
        if(!nearly_equal(expected[i], polyval(p,3,inputs[i]), e)) 
            return false;
    return true;
}

char test_stuck()
{
    double example[20];
    size_t i=0;
    signed char output[20];
    printf("test_stuck... ");
    arange(example, 20);
    for(i=0;i<20;i++)
        output[i] = 1;

    // make a set of stuck values at the beginning
    example[0] = 5.1; /* 1st stuck value */
    example[1] = 5.2; /* 2nd stuck value */
    example[2] = 5.1; /* 3rd stuck value */
    // make a set that fails to be stuck values because num is 3 and the 3rd
    // stuck value is more than reso different than the others
    example[6] = 6.1; /* 1st stuck value */
    example[7] = 6.1; /* 2nd stuck value */
    example[8] = 6.5; /* Would be 3rd stuck value */
    // make a set of stuck values in the middle, longer than num
    example[11] = 2.0; /* 1st stuck value */
    example[12] = 2.1; /* 2nd stuck value */
    example[13] = 2.0; /* 3rd stuck value */
    example[14] = 2.0; /* 4th stuck value */
   // make a set of stuck values at the end
    example[17] = -3.6; /* 1st stuck value */
    example[18] = -3.5; /* 2nd stuck value */
    example[19] = -3.5; /* 3rd stuck value */
    stuck(output, example, 20, 0.2, 3);

    if(all(output, 20))
        return 0;
    for(i=0;i<20;i++) {
        if(i<3 || (i>10 && i<15) || i>16) {
            if(output[i]!=0)
                return 0;
        }
        else {
            if(output[i]!=1)
                return 0;
        }
    }
    return 1;
}

char test_stuck_zero_num()
{
    double example[20];
    size_t i=0;
    signed char output[20];
    printf("test_stuck_zero_num... ");
    arange(example, 20);
    for(i=0;i<20;i++)
        output[i]=1;

    // make a set of stuck values at the end of length 10
    for(i=10;i<20;i++)
        example[i]=5.0;
    stuck(output, example, 20, 1.0, 0);

    if(all(output, 20))
        return 0;
    for(i=0;i<20;i++) {
        if(i<10) {
            if(output[i]!=1)
                return 0;
        }
        else {
            if(output[i]!=0)
                return 0;
        }
    }
    return 1;
}

char test_stuck_neg_num()
{
    double example[10];
    size_t i=0;
    signed char output[10];
    printf("test_stuck_neg_num... ");
    arange(example, 10);
    for(i=0;i<10;i++)
        output[i]=1;

    // make a set of "stuck" values at the beginning of length 3
    for(i=0;i<3;i++)
        example[i]=11.0;
    // make a set of stuck values at the end of length 4
    for(i=6;i<10;i++)
        example[i]=11.0;
    stuck(output, example, 10, 1.0, -4);

    if(all(output,10))
        return 0;
    for(i=0;i<10;i++) {
        if(i<6) {
            if(output[i]!=1)
                return 0;
        }
        else {
            if(output[i]!=0)
                return 0;
        }
    }
    return 1;
}

char test_spike_simple()
{
    double example[20];
    size_t i=0;
    signed char output[20];
    printf("test_spike_simple... ");
    arange(example,20);
    for(i=0;i<20;i++)
        output[i] = 1;
    example[0] = 100.0; /* Make a spike at the beginning */
    example[19] = 100.0; /* Make a spike at the end */
    example[10] = 100.0; /* Make a spike in the middle */
    spike(output, example, 20, 5, 5, 0.1);
    if(all(output,20))
        return 0;
    for(i=0;i<20;i++) {
        if(i==0 || i==19 || i==10) {
            if(output[i]!=0)
                return 0;
        }
        else {
            if(output[i]!=1)
                return 0;
        }
    }
    return 1;
}

char test_spike_l()
{
    double example[20];
    size_t i=0;
    signed char output[20];
    printf("test_spike_l simple... ");
    arange(example,20);
    for(i=0;i<20;i++)
        output[i] = 1;
    example[0] = 100.0; /* Make a spike at the beginning */
    example[19] = 100.0; /* Make a spike at the end */
    example[10] = 100.0; /* Make a spike in the middle */
    spike(output, example, 20, 3, 5, 0.1);
    if(all(output,20))
        return 0;
    for(i=0;i<20;i++) {
        if(i==0 || i==19 || i==10) {
            if(output[i]!=0)
                return 0;
        }
        else {
            if(output[i]!=1)
                return 0;
        }
    }
    return 1;
}

char test_spike_long()
{
    double dat[] = { -1 , 3 , 40 , -1 , 1 , -6 , -6 , 1 , 2 , 4 , 3 , 1 , -1 , 40 , 1 , 1 , 4 , 2 , 2 , 2 , 1 , 2 , 100 };
    signed char expected[] = { 1 , 1 , 1  , 1  , 1 , 1  , 1  , 1 , 1 , 1 , 1 , 1 , 1  , 0  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 };
    size_t len = sizeof(dat)/sizeof(double);
    size_t i=0;
    signed char output[len];
    printf("test_spike_long... ");

    spike(output, dat, len, 7, 5, 0.1);
    for(i=0;i<len;i++) {
        if(expected[i] != output[i]) {
            message = "Expected does not match received.";
            printf("\n");
            print_array(expected, len);
            print_array(output, len);
            return 0;
        }
    }
    return 1;
}

char test_search_sorted()
{
    double a[] = {1, 2, 3, 4, 5};
    double v[] = {-10, 2, 3, 10};
    const size_t len = 4;
    size_t out[len];
    size_t expected[] = {0, 1, 2, 5};
    int i=0;
    memset(out, len * sizeof(size_t), 0);
    printf("test_search_sorted... ");
    search_sorted(out, a, 5, v, len);
    for(i=0;i<len;i++) {
        if(expected[i] != out[i]) {
            message = "Expected does not match received.";
            printf("\n");
            print_array_size_t(expected, len);
            print_array_size_t(out, len);
            return 0;
        }
    }

    return 1;
}

char test_polycal()
{
    double x[] = 
    {    9.97142857,  10.08457143,  10.19542857,  10.304     ,
        10.41028571,  10.51428571,  10.616     ,  10.71542857,
        10.81257143,  10.90742857,  11.        ,  11.09028571,
        11.17828571,  11.264     ,  11.34742857,  11.42857143,
        11.50742857,  11.584     ,  11.65828571,  11.73028571,
        11.8       ,  11.86742857,  11.93257143,  11.99542857,
        12.056     ,  12.11428571,  12.17028571,  12.224     ,
        12.27542857,  12.32457143,  12.37142857,  12.416     ,
        12.45828571,  12.49828571,  12.536     ,  12.57142857,
        12.60457143,  12.63542857,  12.664     ,  12.69028571,
        12.71428571,  12.736     ,  12.75542857,  12.77257143,
        12.78742857,  12.8       ,  12.81028571,  12.81828571,
        12.824     ,  12.82742857,  12.82857143,  12.82742857,
        12.824     ,  12.81828571,  12.81028571,  12.8       ,
        12.78742857,  12.77257143,  12.75542857,  12.736     ,
        12.71428571,  12.69028571,  12.664     ,  12.63542857,
        12.60457143,  12.57142857,  12.536     ,  12.49828571,
        12.45828571,  12.416     ,  12.37142857,  12.32457143,
        12.27542857,  12.224     ,  12.17028571,  12.11428571,
        12.056     ,  11.99542857,  11.93257143,  11.86742857,
        11.8       ,  11.73028571,  11.65828571,  11.584     ,
        11.50742857,  11.42857143,  11.34742857,  11.264     ,
        11.17828571,  11.09028571,  11.        ,  10.90742857,
        10.81257143,  10.71542857,  10.616     ,  10.51428571,
        10.41028571,  10.304     ,  10.19542857,  10.08457143};
    double t[] = 
    { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
     17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
     34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
     51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
     68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
     85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99};
    coeff_vector cals[4];
    double cal0[] = {1.02, 1.0};
    double cal1[] = {1.02, 2.0};
    double cal2[] = {1.02, 3.0};
    double cal3[] = {1.02, 4.0};
    double cal_t[] = {10, 20, 60, 80};
    double out[100];
    const size_t cal_len = 4;
    const size_t x_len = 100;
    double expected[] = {  9.97142857,  10.08457143,  10.19542857,  10.304     ,
        10.41028571,  10.51428571,  10.616     ,  10.71542857,
        10.81257143,  10.90742857,  12.22      ,  12.41209143,
        12.60185143,  12.78928   ,  12.97437714,  13.15714286,
        13.33757714,  13.51568   ,  13.69145143,  13.86489143,
        14.036     ,  14.12977714,  14.22122286,  14.31033714,
        14.39712   ,  14.48157143,  14.56369143,  14.64348   ,
        14.72093714,  14.79606286,  14.86885714,  14.93932   ,
        15.00745143,  15.07325143,  15.13672   ,  15.19785714,
        15.25666286,  15.31313714,  15.36728   ,  15.41909143,
        15.46857143,  15.51572   ,  15.56053714,  15.60302286,
        15.64317714,  15.681     ,  15.71649143,  15.74965143,
        15.78048   ,  15.80897714,  15.83514286,  15.85897714,
        15.88048   ,  15.89965143,  15.91649143,  15.931     ,
        15.94317714,  15.95302286,  15.96053714,  15.96572   ,
        15.96857143,  15.99409143,  16.01728   ,  16.03813714,
        16.05666286,  16.07285714,  16.08672   ,  16.09825143,
        16.10745143,  16.11432   ,  16.11885714,  16.12106286,
        16.12093714,  16.11848   ,  16.11369143,  16.10657143,
        16.09712   ,  16.08533714,  16.07122286,  16.05477714,
        16.036     ,  11.73028571,  11.65828571,  11.584     ,
        11.50742857,  11.42857143,  11.34742857,  11.264     ,
        11.17828571,  11.09028571,  11.        ,  10.90742857,
        10.81257143,  10.71542857,  10.616     ,  10.51428571,
        10.41028571,  10.304     ,  10.19542857,  10.08457143 };
    cals[0].N = 2;
    cals[0].coeff = cal0;
    cals[1].N = 2;
    cals[1].coeff = cal1;
    cals[2].N = 2;
    cals[2].coeff = cal2;
    cals[3].N = 2;
    cals[3].coeff = cal3;
    printf("test_polycal... ");
    polycal(out, cals, cal_t,  cal_len, x, t, x_len);
    // Should be really close but not DBL_EPSILON because of numpy's 
    // lossy representation of the floats
    if( !check_expected(out, expected, x_len, 0.00001, 0)) 
        return 0;
    return 1;
}

char check_expected(double *out, double *expected, size_t len, double atol, double rtol)
{
    size_t i=0;
    for(i=0;i<len;i++) {
        if( fabs(expected[i] - out[i]) > (atol + rtol * fabs(out[i])) ) {
            message = "Expected does not match received.";
            printf("index is %lu\n", i);
            printf("| %.6f - %.6f | >= %.6f\n", out[i], expected[i], (atol + rtol * fabs(out[i])));
            printf("\n");
            print_double_array(expected, len);
            print_double_array(out, len);
            return 0;
        }
    }
    return 1;
}

signed char all(signed char *input, size_t len) 
{
    size_t i=0;
    for(i=0;i<len;i++) {
        if(!input[i]) 
            return 0;
    }
    return 1;
}

void arange(double *arr, size_t len)
{
    size_t i=0;
    for(i=0;i<len;i++) {
        arr[i] = (double) i;
    }
}

void print_array(signed char *array, size_t len)
{
    size_t i=0;
    printf("[");
    for(i=0;i<len-1;i++) {
        printf("%d ", (int)array[i]);
    }
    printf("%d]\n", (int)array[i]);
}


void print_array_size_t(size_t *array, size_t len)
{
    size_t i=0;
    printf("[");
    for(i=0;i<len-1;i++) {
        printf("%lu ", (size_t)array[i]);
    }
    printf("%lu]\n", (size_t)array[i]);
}

void print_double_array(double *array, size_t len)
{
    size_t i=0;
    printf("[");
    for(i=0;i<len-1;i++) {
        printf("%.2f ", (double)array[i]);
    }
    printf("%.2f]\n", (double)array[i]);
}

