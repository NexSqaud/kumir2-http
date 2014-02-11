#ifndef __KUMIR__H
#define __KUMIR__h

#include <wchar.h>

#if __STDC_VERSION__ < 199901L
#   error Requires C99 support! Check your compiler options!
#endif

struct __array__ {
    int dim;
    int lefts[3];
    int rights[3];
    int sizes[3];
    void * data;
    char type;
};

extern void __allocate_array__(struct __array__ * arr);
extern void* __element__(struct __array__ * arr, int i0, int i1, int i2);
extern void __abort__(const wchar_t * message, int lineNo);

extern wchar_t* __string_from_constant__(const wchar_t * str);
extern wchar_t* __concatenate_ss__(const wchar_t * s1, const wchar_t * s2);
extern wchar_t* __concatenate_sc__(const wchar_t * s, wchar_t ch);
extern wchar_t* __concatenate_cs__(wchar_t ch, const wchar_t * s);
extern wchar_t* __concatenate_cc__(wchar_t ch1, wchar_t ch2);

extern wchar_t __get_char_at__(const wchar_t * s, int index);
extern void __set_char_at__(wchar_t * s, int index, wchar_t ch);

extern wchar_t* __get_slice__(const wchar_t * s, int start, int end);
extern void __set_slice_s__(wchar_t ** s, int start, int end, const wchar_t * ss);
extern void __set_slice_c__(wchar_t ** s, int start, int end, wchar_t ch);

extern wchar_t* __string_of_char__(wchar_t ch);

extern void __init_garbage_collector__();
extern void __free_garbage_collector__();

extern double __safe_div_rr__(double a, double b);
extern double __safe_div_ri__(double a, int b);
extern double __safe_div_ir__(int a, double b);
extern double __safe_div_ii__(int a, int b);
extern double __real_power__(double a, double b);
extern int __integer_power__(int a, int b);

extern void __garbage_collector_begin_algorhitm__();
extern void __garbage_collector_register_string__(wchar_t * s);
extern void __garbage_collector_register_array__(struct __array__ * a);
extern void __garbage_collector_set_return_value__(wchar_t * s);
extern void __garbage_collector_set_return_array__(struct __array__ * s);
extern void __garbage_collector_end_algorhitm__();

extern void __input__st_funct(const char * format, int args, ...);
extern void __output__st_funct(const char * format, int args, ...);
extern void __assign_stream__st_funct(const wchar_t * streamname, const wchar_t * filename);

extern void __wait_for_output_queue_flushed__();

extern void __halt__(int lineNo);

#endif