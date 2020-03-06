#include "m_pd.h"
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h> // for rand()
#include <string.h> // for memcpy()
#include <time.h> // to get time for random seeding
#define EXTRAPOINTS 8 // after careful testing, 8 guard points seems safe
#define MAXALGOPARAMS 10
#define NUMALGOS 113
#define BASETEMPO 60
#define MAXTEMPO 240
#define MAXBITDEPTH 32

static const unsigned int paramsPerAlgo[NUMALGOS] =
{
	4,
	5,
	3,
	5,
	5,
	1,
	2,
	2,
	2,
	3,
	4,
	5,
	4,
	5,
	4,
	6,
	4,
	7,
	10,
	6,
	8,
	8,
	10,
	6,
	6,
	3,
	7,
	5,
	7,
	4,
	10,
	9,
	10,
	9,
	10,
	10,
	4,
	3,
	4,
	4,
	7,
	8,
	4,
	5,
	10,
	4,
	5,
	6,
	7,
	6,
	4,
	7,
	9,
	4,
	6,
	10,
	7,
	10,
	10,
	4,
	10,
	6,
	7,
	8,
	2,
	4,
	3,
	3,
	4,
	3,
	5,
	4,
	4,
	4,
	4,
	5,
	4,
	5,
	4,
	4,
	6,
	7,
	4,
	4,
	4,
	4,
	6,
	7,
	4,
	5,
	4,
	4,
	4,
	6,
	7,
	6,
	6,
	5,
	6,
	6,
	5,
	9,
	6,
	10,
	7,
	7,
	8,
	6,
	7,
	6,
	5,
	5,
	5
};

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
    unsigned int x_paramsPerAlgo[NUMALGOS];
    double *x_signalBuffer;
    t_outlet *x_outletTime;
    t_outlet *x_outletMu;
    t_outlet *x_outletParamsPerAlgo;
    t_outlet *x_outletWrapBang;
    
} outputAlgo_tilde;
