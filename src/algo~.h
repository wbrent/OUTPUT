#include "m_pd.h"
#include "expr.h" // from https://github.com/silvioprog/expr
// #include <math.h> // already included with expr.h
// #include <limits.h> // already included with expr.h
// #include <stdlib.h> // for rand() // already included with expr.h
// #include <string.h> // for memcpy() // already included with expr.h

#define EXTRAPOINTS 8 // after careful testing, 8 guard points seems safe
#define MAXALGOPARAMS 20
#define DEFAULTSAMPLERATE 44100
#define MAXSAMPLERATE 176400 // to increase this, we would need to increase EXTRAPOINTS from 8, but this causes too much extra overhead.
#define MAXBITDEPTH 32
#define NUMALGOSETTINGS 6 // algo, bit-depth, sample rate, time, time loop start, time loop end
#define ARRAY36364689SIZE 256
#define ALGOTILDEVERSION "0.9.11"

// this was the output of "36364689"[i] for i=0:255 one day on my computer. it's undefined what comes out past i=7, but I liked the results so I'm recording them here in a specific array that can produce defined behavior.
static const uint32_t array36364689[ARRAY36364689SIZE] =
{
51, 54, 51, 54, 52, 54, 56, 57, 0, 37, 115, 58, 32, 116, 101, 109, 112, 111, 58, 32, 37, 48, 46, 50, 102, 0, 37, 115, 58, 32, 116, 101, 109, 112, 111, 70, 97, 99, 116, 111, 114, 58, 32, 37, 48, 46, 54, 102, 0, 37, 115, 58, 32, 98, 105, 116, 68, 101, 112, 116, 104, 58, 32, 37, 48, 46, 54, 102, 0, 37, 115, 58, 32, 105, 110, 116, 101, 114, 112, 111, 108, 97, 116, 105, 111, 110, 58, 32, 37, 117, 0, 37, 115, 58, 32, 97, 108, 103, 111, 32, 99, 104, 111, 105, 99, 101, 58, 32, 37, 117, 0, 37, 115, 58, 32, 112, 97, 114, 97, 109, 115, 58, 32, 91, 0, 37, 117, 44, 32, 0, 37, 117, 93, 0, 37, 115, 58, 32, 116, 58, 32, 37, 117, 0, 0, 37, 115, 58, 32, 115, 97, 109, 112, 108, 105, 110, 103, 32, 114, 97, 116, 101, 58, 32, 37, 105, 0, 37, 115, 58, 32, 98, 108, 111, 99, 107, 32, 115, 105, 122, 101, 58, 32, 37, 105, 0, 119, 0, 37, 115, 58, 32, 102, 97, 105, 108, 101, 100, 32, 116, 111, 32, 99, 114, 101, 97, 116, 101, 32, 37, 115, 0, 37, 117, 10, 0, 37, 48, 46, 54, 102, 10, 0, 42, 42, 42, 42, 32, 78, 69, 87, 66, 76, 79, 67, 75, 32, 42, 42, 42, 42, 0, 116, 66, 108, 111, 99, 107, 83, 116, 97, 114, 116, 58, 32, 37
};

// NOTE: if MAXALGOPARAMS changes, these contents must change accordingly
const char *paramStrings[MAXALGOPARAMS] = {"p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "p16", "p17", "p18", "p19"};

static expr_num_t userFuncIntCast(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  int x = expr_eval(&vec_nth(args, 0));
  (void) f, (void) c;

  result = x;
  return result;
}

static expr_num_t userFuncFloor(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  uint32_t x = expr_eval(&vec_nth(args, 0));
  (void) f, (void) c;

  result = x;
  return result;
}

static expr_num_t userFuncCeil(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  uint32_t x = expr_eval(&vec_nth(args, 0)) + 1;
  (void) f, (void) c;

  result = x;
  return result;
}

static expr_num_t userFuncRound(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  uint32_t x = expr_eval(&vec_nth(args, 0)) + 0.5;
  (void) f, (void) c;

  result = x;
  return result;
}

static expr_num_t userFuncLookup(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  int len;
  uint32_t bufLen = 21; // NOTE: the max number of digits of an unsigned long long is 20. buffer must be one byte larger than the number of digits in s, to have space for the terminating null character

  expr_num_t s = expr_eval(&vec_nth(args, 0));
  uint32_t i = expr_eval(&vec_nth(args, 1));
  (void) f, (void) c;
  char buffer[bufLen];

  len = snprintf(buffer, bufLen, "%llu", (unsigned long long)s);
  i = i%len; // wrap the index at the actual string length

  if(len > 0 && len < bufLen)
    result = buffer[i];
  else
    result = 0;

  return result;
}

static expr_num_t userFuncIfElse(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  expr_num_t x = expr_eval(&vec_nth(args, 0));
  expr_num_t y = expr_eval(&vec_nth(args, 1));
  expr_num_t z = expr_eval(&vec_nth(args, 2));
  (void) f, (void) c;

  result = (x > 0) ? y : z;
  return result;
}

static expr_num_t userFuncSin(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  expr_num_t x = expr_eval(&vec_nth(args, 0));
  (void) f, (void) c;

  result = sin(x);
  return result;
}

static expr_num_t userFuncAbs(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  expr_num_t x = expr_eval(&vec_nth(args, 0));
  (void) f, (void) c;

  result = fabs(x);
  return result;
}

static expr_num_t userFuncBitwiseNot(struct expr_func *f, vec_expr_t *args, void *c) {
  expr_num_t result;
  uint32_t x = expr_eval(&vec_nth(args, 0));
  (void) f, (void) c;

  result = ~x;
  return result;
}

static struct expr_func exprUserfuncs[] = {
  {"int", userFuncIntCast, NULL, 0},
  {"floor", userFuncFloor, NULL, 0},
  {"ceil", userFuncCeil, NULL, 0},
  {"round", userFuncRound, NULL, 0},
  {"lookup", userFuncLookup, NULL, 0},
  {"ifElse", userFuncIfElse, NULL, 0},
  {"sin", userFuncSin, NULL, 0},
  {"abs", userFuncAbs, NULL, 0},
  {"bitNot", userFuncBitwiseNot, NULL, 0},
  {NULL, NULL, NULL, 0}
};

typedef enum {backward = -1, forward = 1} t_timeDir;
typedef enum {false = 0, true} t_bool;

static t_class *algo_tilde_class;

typedef struct _algo_tilde
{
    t_object x_obj;
	  t_symbol *x_objSymbol;
    t_canvas *x_canvas;
	  t_clock *x_clock;
    t_bool x_startupFlag;
    double x_sr;
    double x_n;
    double x_bitDepth;
    double x_quantSteps;
    unsigned char x_interpSwitch;
    unsigned char x_computeSwitch;
	  t_timeDir x_timeDirection;
    t_bool x_presetLoadParams;
    t_bool x_presetLoadBitDepth;
    t_bool x_presetLoadSampleRate;
    t_bool x_presetLoadTime;
    t_bool x_presetLoadLoopPoints;
    unsigned char x_debug;

    uint32_t x_t;
    uint32_t x_tBlockStart;
    uint32_t x_tBlockEnd;
    uint32_t x_tLoopPoints[2];
    double x_mu;
    double x_incr;
    double x_samplerate;
    double x_sampIdx;
    char *x_paramStrings[MAXALGOPARAMS];
    uint32_t x_params[MAXALGOPARAMS];
    uint32_t x_numAlgoParams;
    uint32_t x_array36364689[ARRAY36364689SIZE];
    double *x_signalBuffer;

    struct expr *x_exprExp;
    char *x_exprStr; // cannot be const char * because the expression string changes
    struct expr_var_list x_exprVars;

    t_outlet *x_outletTime;
    t_outlet *x_outletNumAlgoParams;
    t_outlet *x_outletAlgoSettings;
    t_outlet *x_outletWrapBang;

} algo_tilde;
