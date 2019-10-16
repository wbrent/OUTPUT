/*

outputAlgo~

Copyright 2019 William Brent

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.


version 0.4.1, October 11, 2019

next, should interpolate tempo, bit depth, and p-parameters per sample to reduce clicks on changes

is getTimeIndex/setTimeIndex getting and setting x_t on block boundaries only? that seems to be happening when testing the patch, but need to think that through. could make tBlockStart in _perform routine a variable in the object dataspace and use that for getting/setting since it's a known quantity at the start of each block, not a potentially changing quantity mid-block.

getting double wrap-bangs

*/

#include "m_pd.h"
#include <math.h>
#include <float.h>
#include <limits.h>
#define EXTRAPOINTS 8 // after careful testing, 8 guard points seems safe
#define ALGOZEROPARAMS 4
#define ALGOONEPARAMS 5
#define ALGOTWOPARAMS 3
#define ALGOTHREEPARAMS 5
#define ALGOFOURPARAMS 5
#define MAXALGOPARAMS 10
#define NUMALGOS 5
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
    unsigned char x_bitDepth;
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


/* ------------------------ outputAlgo~ -------------------------------- */
static double outputAlgo_tilde_cubicInterpolate(double y0, double y1, double y2, double y3, double mu)
{
   double a0, a1, a2, a3, mu2;

   mu2 = mu*mu;
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}

static void outputAlgo_tilde_print(outputAlgo_tilde *x)
{
	int i;
	
	post("%s: tempo: %0.2f", x->x_objSymbol->s_name, x->x_tempo);
	post("%s: tempoFactor: %0.6f", x->x_objSymbol->s_name, x->x_incr);
	post("%s: bitDepth: %u", x->x_objSymbol->s_name, x->x_bitDepth);
	post("%s: interpolation: %u", x->x_objSymbol->s_name, x->x_interpSwitch);
	
	post("%s: algo choice: %u", x->x_objSymbol->s_name, x->x_algoChoice);
	
	startpost("%s: params: [", x->x_objSymbol->s_name);
	
	for(i=0; i<MAXALGOPARAMS-1; i++)
		startpost("%u, ", x->x_params[i]);
	
	startpost("%u]", x->x_params[MAXALGOPARAMS-1]);
	endpost();

	post("%s: t: %u", x->x_objSymbol->s_name, x->x_t);
	
	post("");
	post("%s: sampling rate: %i", x->x_objSymbol->s_name, (int)x->x_sr);
	post("%s: block size: %i", x->x_objSymbol->s_name, (int)x->x_n);
	post("");
}

static void outputAlgo_tilde_bitDepth(outputAlgo_tilde *x, t_floatarg b)
{
	// keep bounded within 1 and MAXBITDEPTH
	b = (b<1)?1:b;
	b = (b>MAXBITDEPTH)?MAXBITDEPTH:b;
	x->x_bitDepth = b;
 	x->x_quantSteps = pow(2, x->x_bitDepth);
}

static void outputAlgo_tilde_tempo(outputAlgo_tilde *x, t_floatarg t)
{	
	// keep bounded within 1 and MAXTEMPO
	t = (t<1.0)?1.0:t;
	t = (t>MAXTEMPO)?MAXTEMPO:t;
	x->x_tempo = t;
	
	// calculate the interpolation increment (tempo factor) relative to BASETEMPO BPM
	x->x_incr = (x->x_tempo/(double)BASETEMPO);
}

static void outputAlgo_tilde_getTimeIndex(outputAlgo_tilde *x)
{
	outlet_float(x->x_outletTime, x->x_tBlockEnd);
}

static void outputAlgo_tilde_setTimeIndex(outputAlgo_tilde *x, t_floatarg t)
{
	// keep bounded within 1 and UINT_MAX
	t = (t<0)?0:t;
	t = (t>UINT_MAX)?UINT_MAX:t;
	x->x_t = t;
}

static void outputAlgo_tilde_getInterpMu(outputAlgo_tilde *x)
{
	float mu;
	
	// probably due to rounding error, posting sampIdx and its floor shows a different integer in the case that sampIdx = 44.0, for example.
	mu = x->x_sampIdx - floor(x->x_sampIdx);
	// ensure that this wraps at 1.0 no matter what	
	// strangely, have to do this in float precision. doesn't work as expected in double precision
	mu = (mu>=1.0f)?0.0f:mu;
	
	outlet_float(x->x_outletMu, mu);
}

static void outputAlgo_tilde_setInterpMu(outputAlgo_tilde *x, t_floatarg m)
{
	// keep bounded within 0 and 1
	m = (m<0)?0:m;
	m = (m>1.0f)?1.0f:m;
	x->x_sampIdx = m;
}

static void outputAlgo_tilde_interpSwitch(outputAlgo_tilde *x, t_floatarg i)
{
	// keep bounded within 0 and 1
	i = (i<0)?0:i;
	i = (i>1)?1:i;
	x->x_interpSwitch = i;
}

static void outputAlgo_tilde_computeSwitch(outputAlgo_tilde *x, t_floatarg c)
{
	// keep bounded within 0 and 1
	c = (c<0)?0:c;
	c = (c>1)?1:c;
	x->x_computeSwitch = c;
}

static void outputAlgo_tilde_algoChoice(outputAlgo_tilde *x, t_floatarg a)
{
	// keep bounded within 0 and (NUMALGOS-1)
	a = (a<0)?0:a;
	a = (a>(NUMALGOS-1))?(NUMALGOS-1):a;
	x->x_algoChoice = a;
}

static void outputAlgo_tilde_debug(outputAlgo_tilde *x, t_floatarg d)
{
	// keep bounded within 0 and 255
	d = (d<0)?0:d;
	d = (d>255)?255:d;
	x->x_debug = d;
}

static void outputAlgo_tilde_parameters(outputAlgo_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	
	for(i=0; i<argc; i++)
		x->x_params[i] = atom_getfloat(argv+i);
		
	// if there weren't enough arguments, fill remaining params with 0
	for(; i<MAXALGOPARAMS; i++)
		x->x_params[i] = 0;
}

static void *outputAlgo_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    outputAlgo_tilde *x = (outputAlgo_tilde *)pd_new(outputAlgo_tilde_class);
	int i;
	
	outlet_new(&x->x_obj, &s_signal);
    x->x_outletTime = outlet_new(&x->x_obj, &s_float);
    x->x_outletMu = outlet_new(&x->x_obj, &s_float);
    x->x_outletWrapBang = outlet_new(&x->x_obj, &s_bang);

	// store the pointer to the symbol containing the object name. Can access it for error and post functions via s->s_name
	x->x_objSymbol = s;

	x->x_sr = 44100.0f;
	x->x_n = 64.0f;

	x->x_bitDepth = 24;
	x->x_quantSteps = pow(2, x->x_bitDepth);
	x->x_interpSwitch = 1;
	x->x_algoChoice = 0;
	x->x_computeSwitch = 1;
	x->x_debug = 0;
	
	x->x_t = 0;
	x->x_tBlockStart = 0;
	x->x_tBlockEnd = 0;
	x->x_mu = 0.0f;
	x->x_incr = 0.0f;
	x->x_sampIdx = 0.0f;
	
	x->x_tempo = 60.0f;
	outputAlgo_tilde_tempo(x, x->x_tempo);

	x->x_signalBuffer = (t_sample *)t_getbytes((x->x_n*4+EXTRAPOINTS) * sizeof(t_sample));
	
	for(i=0; i<x->x_n*4+EXTRAPOINTS; i++)
		x->x_signalBuffer[i] = 0.0;
		
	for(i=0; i<argc; i++)
		x->x_params[i] = atom_getfloat(argv+i);
		
	// if there weren't enough arguments, fill remaining params with 0
	for(; i<MAXALGOPARAMS; i++)
		x->x_params[i] = 0;
	
    return(x);
}


static t_int *outputAlgo_tilde_perform(t_int *w)
{
    unsigned int i, j, n, m, hop;
    unsigned int p0, p1, p2, p3, p4, p5, p6, p7, p8, p9;
	
    outputAlgo_tilde *x = (outputAlgo_tilde *)(w[1]);
    t_sample *out = (t_float *)(w[2]);
    n = w[3];
	
	// the rough number of algo samples to compute (m) is the blocksize (n) scaled by the tempo factor (increment). we round to make sure we generate more than needed
	m = round(n*x->x_incr);
	
	// note the current time index at the start of the block
	x->x_tBlockStart = x->x_t;
	
	// grab the params from the array before the for loop, since those won't change within a block anyway
	p0 = x->x_params[0];
	p1 = x->x_params[1];
	p2 = x->x_params[2];
	p3 = x->x_params[3];
	p4 = x->x_params[4];
	p5 = x->x_params[5];
	p6 = x->x_params[6];
	p7 = x->x_params[7];
	p8 = x->x_params[8];
	p9 = x->x_params[9];
	
	if(x->x_computeSwitch)
	{
		double sampIdxBlockEnd;
			
		if(x->x_debug)
		{
			post("**** NEWBLOCK ****");
			post("tBlockStart: %u, m: %i, incr: %f", x->x_tBlockStart, m, x->x_incr);
			post("0: [%0.6f], tMinusOne: %lu", x->x_signalBuffer[0], x->x_t-1);
			// don't decrement debug until end of perform routine
		}
		
		// calculate this block of the algo signal, plus extra guard points
		for(i=1; i<m+EXTRAPOINTS; i++)
		{
			t_sample thisSampleFloat;
			unsigned long int thisSample;
				
			// perform the algorithm in unsigned int precision. Bitwise operators MUST BE used in UINT precision. we store the result in higher precision - unsigned long int
			switch(x->x_algoChoice)
			{
				case 0:
					thisSample = x->x_t*((x->x_t >> p0 | x->x_t >> p1) & p2 & x->x_t >> p3);
					break;
				case 1:
					thisSample = x->x_t*(p0&x->x_t>>p1)|((x->x_t*p2)*(x->x_t>>p3)*(x->x_t<<p4));
					break;
				case 2:
					thisSample = ((x->x_t*p0)*(x->x_t>>p1)|(x->x_t<<p2));
					break;
				case 3:
					thisSample = x->x_t*((x->x_t>>p0)*(x->x_t>>p1))|((x->x_t>>p2)&(p3&x->x_t>>p4));
					break;
				case 4:
					thisSample = (x->x_t*p0&(x->x_t>>p1))|((x->x_t*p2)&(x->x_t*p3>>p4));
					break;
				default:
					thisSample = 0;
					break;					
			}
		
			// convert the unsigned long long int-ranged sample into a float sample
			thisSampleFloat = thisSample / (t_float)x->x_quantSteps;
			thisSample = floor(thisSampleFloat);
			thisSampleFloat = thisSampleFloat - thisSample;
			thisSampleFloat = thisSampleFloat*2.0-1.0;
			x->x_signalBuffer[i] = thisSampleFloat;

			if(x->x_debug)
			{
				post("%i: [%0.6f], t: %lu", i, x->x_signalBuffer[i], x->x_t);
				// don't decrement debug until end of perform routine
			}

			// advance the time variable t, which maxes out at UINT_MAX
			x->x_t = (x->x_t + 1) % UINT_MAX;
		}
	
		// to start the block, sampIdx should be 1.mu from last time
		x->x_sampIdx = (x->x_sampIdx - floor(x->x_sampIdx)) + 1.0;
		
		if(x->x_debug)
			post("pre-output sampIdx: %f", x->x_sampIdx);

		// interpolate and output
		for(i=0; i<n; i++)
		{
			if(x->x_interpSwitch)
				x->x_mu = x->x_sampIdx - floor(x->x_sampIdx);
			else
				x->x_mu = 0.0;
				
			j = floor(x->x_sampIdx);
		
			out[i] = outputAlgo_tilde_cubicInterpolate(x->x_signalBuffer[j-1], x->x_signalBuffer[j], x->x_signalBuffer[j+1], x->x_signalBuffer[j+2], x->x_mu);

			if(x->x_debug)
			{
				post("%i: [%0.6f, %0.6f, %0.6f, %0.6f][%0.6f, %0.6f], mu: %f, sampIdx: %f, j: %i, jInterp: %f, [%0.6f]", i, x->x_signalBuffer[j-1], x->x_signalBuffer[j], x->x_signalBuffer[j+1], x->x_signalBuffer[j+2], x->x_signalBuffer[j+3], x->x_signalBuffer[j+4], x->x_mu, x->x_sampIdx, j, j+x->x_mu, out[i]);
				// don't decrement debug until end of perform routine
			}

			sampIdxBlockEnd = x->x_sampIdx;
			x->x_sampIdx += x->x_incr;
		}

		if(x->x_debug)
		{
			post("sampIdxBlockEnd: %f", sampIdxBlockEnd);
			post("post-output sampIdx: %f", x->x_sampIdx);
		}

		// at this point, if we take floor of x_sampIdx - 1, we know how many samples we have moved through in the interpolation loop of n samples above. this is how much we should move forward in time, and this is also the sample in the signal buffer we should move to the head (index 0) of the next signal block.
		hop = floor(x->x_sampIdx - 1.0);

		// store the final sample of the algo signal block to index 0 of the signal buffer for next time. since we generated extra samples past the m samples we actually want to hear, we need to rewind the time index. the time index for next block should be m samples past the time index from the start of this block. remember to wrap at UINT_MAX as our maximum time index.
		x->x_signalBuffer[0] = x->x_signalBuffer[hop];
		x->x_t = (x->x_tBlockStart+hop) % UINT_MAX;
		x->x_tBlockEnd = x->x_t;

		if(x->x_debug)
		{
			if(floor(sampIdxBlockEnd) == floor(x->x_sampIdx))
				post("staying on current sample");
			else
				post("moved to next sample");

			post("hop: %u", hop);
			post("t for sample 1 of next block: %lu", x->x_t);
			post("sample m [%i] to head for next block: %0.6f", hop, x->x_signalBuffer[0]);
			post("");
			// don't decrement debug until end of perform routine
		}

		// bang if the next time point has wrapped, indicating pattern reset
		// this can double-trigger if the wrap point happens to align in a certain way relative to block size. would have to add a flag variable to the object dataspace to prevent this
		if(x->x_t < x->x_tBlockStart)
			outlet_bang(x->x_outletWrapBang);
 	}
 	else
 	{
		for(i=0; i<n; i++)
			out[i] = 0.0;
 	}

	if(x->x_debug)
		x->x_debug--;

    return (w+4);
}


static void outputAlgo_tilde_dsp(outputAlgo_tilde *x, t_signal **sp)
{
	dsp_add(
		outputAlgo_tilde_perform,
		3,
		x,
		sp[0]->s_vec,
		sp[0]->s_n
	);

	if(x->x_n != sp[0]->s_n)
	{
		x->x_signalBuffer = (t_sample *)t_resizebytes(x->x_signalBuffer, (x->x_n*4+EXTRAPOINTS) * sizeof(t_sample), (sp[0]->s_n*4+EXTRAPOINTS) * sizeof(t_sample));

		x->x_n = sp[0]->s_n;
	}
	
	if(x->x_sr != sp[0]->s_sr)
		x->x_sr = sp[0]->s_sr;
};


static void outputAlgo_tilde_free(outputAlgo_tilde *x)
{	
	// free the signalBuffer memory
	t_freebytes(x->x_signalBuffer,  (x->x_n*4+EXTRAPOINTS)*sizeof(t_sample));
};


void outputAlgo_tilde_setup(void)
{
    outputAlgo_tilde_class = 
    class_new(
    	gensym("outputAlgo~"),
    	(t_newmethod)outputAlgo_tilde_new,
    	(t_method)outputAlgo_tilde_free,
        sizeof(outputAlgo_tilde),
        CLASS_DEFAULT,
        A_GIMME,
		0
    );

	class_addcreator(
		(t_newmethod)outputAlgo_tilde_new,
		gensym("output/outputAlgo~"),
		A_GIMME,
		0
	);

	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_bitDepth,
		gensym("bitDepth"),
		A_DEFFLOAT,
		0
	);
	
	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_tempo,
		gensym("tempo"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_algoChoice,
		gensym("algoChoice"),
		A_DEFFLOAT,
		0
	);
	
	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_parameters,
		gensym("parameters"),
		A_GIMME,
		0
	);

	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_getTimeIndex,
		gensym("getTimeIndex"),
		0
	);
	
	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_setTimeIndex,
		gensym("setTimeIndex"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_getInterpMu,
		gensym("getInterpMu"),
		0
	);
	
	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_setInterpMu,
		gensym("setInterpMu"),
		A_DEFFLOAT,
		0
	);
	
	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_interpSwitch,
		gensym("interpolate"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_computeSwitch,
		gensym("compute"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_debug,
		gensym("debug"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		outputAlgo_tilde_class,
		(t_method)outputAlgo_tilde_print,
		gensym("print"),
		0
	);

    class_addmethod(
    	outputAlgo_tilde_class,
    	(t_method)outputAlgo_tilde_dsp,
    	gensym("dsp"),
    	0
    );
}