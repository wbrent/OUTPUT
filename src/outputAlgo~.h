#include "m_pd.h"
#include <math.h>
#include <float.h>
#include <limits.h>
#define EXTRAPOINTS 8 // after careful testing, 8 guard points seems safe
#define MAXALGOPARAMS 10
#define NUMALGOS 113
#define BASETEMPO 60
#define MAXTEMPO 240
#define MAXBITDEPTH 32

static t_class *outputAlgo_tilde_class;

typedef struct _outputAlgo_tilde 
{
    t_object x_obj;
	t_symbol *x_objSymbol;
    t_float x_sr;
    t_float x_n;
    t_float x_bitDepth;
    unsigned long int x_quantSteps;
    unsigned char x_interpSwitch;
    unsigned char x_computeSwitch;
    unsigned char x_debug;

    unsigned int x_t;
    unsigned int x_tBlockStart;
    unsigned int x_tBlockEnd;
    double x_mu;
    double x_incr;
    double x_sampIdx;
    t_float x_tempo;
    unsigned char x_algoChoice;
    unsigned int x_params[MAXALGOPARAMS];
    t_sample *x_signalBuffer;
    t_outlet *x_outletTime;
    t_outlet *x_outletMu;
    t_outlet *x_outletWrapBang;
    
} outputAlgo_tilde;
