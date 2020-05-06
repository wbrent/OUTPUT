#include "m_pd.h"
#include "exprDouble.h" // modified from expr.h at https://github.com/zserge/expr
// #include <math.h> // already included with exprDouble.h
// #include <limits.h> // already included with exprDouble.h
// #include <stdlib.h> // for rand() // already included with exprDouble.h
// #include <string.h> // for memcpy() // already included with exprDouble.h

#define EXTRAPOINTS 8 // after careful testing, 8 guard points seems safe
#define MAXALGOPARAMS 20
#define BASETEMPO 60
#define DEFAULTSAMPLERATE 44100
#define MAXTEMPO 240
#define MAXBITDEPTH 32
#define NUMALGOSETTINGS 26 // algo, 20 params, bit-depth, tempo, time, time loop start, time loop end. NOTE: if MAXALGOPARAMS changes, this must change accordingly
#define ARRAY36364689SIZE 256
#define ALGOTILDEVERSION "0.9.3"

// this was the output of "36364689"[i] for i=0:255 one day on my computer. it's undefined what comes out past i=7, but I liked the results so I'm recording them here in a specific array that can produce defined behavior.
static const unsigned int array36364689[ARRAY36364689SIZE] = 
{
51, 54, 51, 54, 52, 54, 56, 57, 0, 37, 115, 58, 32, 116, 101, 109, 112, 111, 58, 32, 37, 48, 46, 50, 102, 0, 37, 115, 58, 32, 116, 101, 109, 112, 111, 70, 97, 99, 116, 111, 114, 58, 32, 37, 48, 46, 54, 102, 0, 37, 115, 58, 32, 98, 105, 116, 68, 101, 112, 116, 104, 58, 32, 37, 48, 46, 54, 102, 0, 37, 115, 58, 32, 105, 110, 116, 101, 114, 112, 111, 108, 97, 116, 105, 111, 110, 58, 32, 37, 117, 0, 37, 115, 58, 32, 97, 108, 103, 111, 32, 99, 104, 111, 105, 99, 101, 58, 32, 37, 117, 0, 37, 115, 58, 32, 112, 97, 114, 97, 109, 115, 58, 32, 91, 0, 37, 117, 44, 32, 0, 37, 117, 93, 0, 37, 115, 58, 32, 116, 58, 32, 37, 117, 0, 0, 37, 115, 58, 32, 115, 97, 109, 112, 108, 105, 110, 103, 32, 114, 97, 116, 101, 58, 32, 37, 105, 0, 37, 115, 58, 32, 98, 108, 111, 99, 107, 32, 115, 105, 122, 101, 58, 32, 37, 105, 0, 119, 0, 37, 115, 58, 32, 102, 97, 105, 108, 101, 100, 32, 116, 111, 32, 99, 114, 101, 97, 116, 101, 32, 37, 115, 0, 37, 117, 10, 0, 37, 48, 46, 54, 102, 10, 0, 42, 42, 42, 42, 32, 78, 69, 87, 66, 76, 79, 67, 75, 32, 42, 42, 42, 42, 0, 116, 66, 108, 111, 99, 107, 83, 116, 97, 114, 116, 58, 32, 37
};

// NOTE: if MAXALGOPARAMS changes, these contents must change accordingly
const char *paramStrings[MAXALGOPARAMS] = {"p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "p16", "p17", "p18", "p19"};

static struct expr_func exprUserfuncs[] = {{NULL, NULL, NULL, 0}};

typedef enum {backward = -1, forward = 1} t_timeDir;

static t_class *algo_tilde_class;

typedef struct _algo_tilde 
{
    t_object x_obj;
	t_symbol *x_objSymbol;
    t_canvas *x_canvas;
	t_clock *x_clock;
    unsigned char x_startupFlag;
    t_float x_sr;
    t_float x_n;
    double x_bitDepth;
    double x_quantSteps;
    unsigned char x_interpSwitch;
    unsigned char x_computeSwitch;
	t_timeDir x_timeDirection;
    unsigned char x_debug;

    unsigned int x_t;
    unsigned int x_tBlockStart;
    unsigned int x_tBlockEnd;
    unsigned int x_tLoopPoints[2];
    double x_mu;
    double x_incr;
    double x_sampIdx;
    double x_tempo;
    char *x_paramStrings[MAXALGOPARAMS];
    unsigned int x_params[MAXALGOPARAMS];
    unsigned int x_numAlgoParams;
    unsigned int x_array36364689[ARRAY36364689SIZE];
    double *x_signalBuffer;
    
    struct expr* x_exprExp;
    char *x_exprStr; // cannot be const char * because the expression string changes
    struct expr_var_list x_exprVars;
    
    t_outlet *x_outletTime;
    t_outlet *x_outletNumAlgoParams;
    t_outlet *x_outletAlgoSettings;
    t_outlet *x_outletWrapBang;
    
} algo_tilde;
