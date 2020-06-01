/*

algo~

Copyright 2019 William Brent

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.


version 0.9.8, June 1, 2020

*/

#include "algo~.h"

/* ------------------------ algo~ -------------------------------- */
static double algo_tilde_cubicInterpolate(double y0, double y1, double y2, double y3, double mu)
{
   double a0, a1, a2, a3, mu2;

   mu2 = mu*mu;
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}

static void algo_tilde_print(algo_tilde *x)
{
	uint32_t i;

	post("%s version %s", x->x_objSymbol->s_name, ALGOTILDEVERSION);

	post("%s: algorithm: %s", x->x_objSymbol->s_name, x->x_exprStr);

	startpost("%s: params: [", x->x_objSymbol->s_name);

	if(x->x_numAlgoParams > 0)
	{
		for(i=0; i<x->x_numAlgoParams-1; i++)
			startpost("%u, ", x->x_params[i]);

		startpost("%u]", x->x_params[x->x_numAlgoParams-1]);
		endpost();
	}
	else
	{
		startpost("]");
		endpost();
	}

	post("%s: numAlgoParams: %u", x->x_objSymbol->s_name, x->x_numAlgoParams);
	post("%s: bitDepth: %0.15f", x->x_objSymbol->s_name, x->x_bitDepth);
	post("%s: samplerate: %0.15f", x->x_objSymbol->s_name, x->x_samplerate);
	post("%s: sample increment: %0.15f", x->x_objSymbol->s_name, x->x_incr);
	post("%s: t: %u", x->x_objSymbol->s_name, x->x_t);
	post("%s: time loop points: [%u, %u]", x->x_objSymbol->s_name, x->x_tLoopPoints[0], x->x_tLoopPoints[1]);
	post("%s: interpolation: %s", x->x_objSymbol->s_name, (x->x_interpSwitch > 0) ? "ON" : "OFF");
	post("%s: compute: %s", x->x_objSymbol->s_name, (x->x_computeSwitch > 0) ? "ON" : "OFF");
	post("%s: time direction: %s", x->x_objSymbol->s_name, (x->x_timeDirection==forward) ? "forward" : "backward");

  post("%s: Load parameters: %s", x->x_objSymbol->s_name, (x->x_presetLoadParams > 0) ? "TRUE" : "FALSE");
  post("%s: Load bitDepth: %s", x->x_objSymbol->s_name, (x->x_presetLoadBitDepth > 0) ? "TRUE" : "FALSE");
  post("%s: Load samplerate: %s", x->x_objSymbol->s_name, (x->x_presetLoadSampleRate > 0) ? "TRUE" : "FALSE");
  post("%s: Load time: %s", x->x_objSymbol->s_name, (x->x_presetLoadTime > 0) ? "TRUE" : "FALSE");
  post("%s: Load loop points: %s", x->x_objSymbol->s_name, (x->x_presetLoadLoopPoints > 0) ? "TRUE" : "FALSE");

  post("%s: Pd sampling rate: %i", x->x_objSymbol->s_name, (int)x->x_sr);
	post("%s: block size: %i", x->x_objSymbol->s_name, (int)x->x_n);
	post("");
}

static void algo_tilde_getAlgoSettings(algo_tilde *x)
{
	int i;
	t_atom *listOut;

	listOut = (t_atom *)t_getbytes((MAXALGOPARAMS+NUMALGOSETTINGS)*sizeof(t_atom));

	// exprStr is a char*, but must be a t_symbol* for SETSYMBOL. use gensym()
	SETSYMBOL(listOut, gensym(x->x_exprStr));

	for(i=0; i<MAXALGOPARAMS; i++)
		SETFLOAT(listOut+1+i, x->x_params[i]);

	// i is 20 here

	// using post-for-loop value of i adapts to whatever MAXALGOPARAMS is
	SETFLOAT(listOut+i+1, x->x_bitDepth);
	SETFLOAT(listOut+i+2, x->x_samplerate);
	SETFLOAT(listOut+i+3, x->x_tBlockEnd);
	SETFLOAT(listOut+i+4, x->x_tLoopPoints[0]);
	SETFLOAT(listOut+i+5, x->x_tLoopPoints[1]);

	outlet_list(x->x_outletAlgoSettings, 0, MAXALGOPARAMS+NUMALGOSETTINGS, listOut);

	// free local memory
	t_freebytes(listOut, (MAXALGOPARAMS+NUMALGOSETTINGS) * sizeof(t_atom));
}

static void algo_tilde_getTimeIndex(algo_tilde *x)
{
	t_atom *listOut;
	float mu;

	listOut = (t_atom *)t_getbytes(2*sizeof(t_atom));

	// probably due to rounding error, posting sampIdx and its floor shows a different integer in the case that sampIdx = 44.0, for example.
	mu = x->x_sampIdx - floor(x->x_sampIdx);
	// ensure that this wraps at 1.0 no matter what
	// strangely, have to do this in float precision. doesn't work as expected in double precision
	mu = (mu>=1.0f)?0.0f:mu;

	SETFLOAT(listOut, x->x_tBlockEnd);
	SETFLOAT(listOut+1, mu);

	outlet_list(x->x_outletTime, 0, 2, listOut);

	// free local memory
	t_freebytes(listOut, 2 * sizeof(t_atom));
}

static void algo_tilde_getNumAlgoParams(algo_tilde *x)
{
	outlet_float(x->x_outletNumAlgoParams, x->x_numAlgoParams);
}

static void algo_tilde_setTimeIndex(algo_tilde *x, t_floatarg t, t_floatarg m)
{
	// keep bounded within 1 and UINT_MAX
	t = (t<1)?1:t;
	t = (t>UINT_MAX)?UINT_MAX:t;
	x->x_t = t; // best to set x->x_t directly, because x->x_tBlockStart grabs that at the beginning of the next block

	// keep bounded within 0 and 1
	m = (m<0)?0:m;
	m = (m>1.0f)?1.0f:m;
	x->x_sampIdx = m;
}

static void algo_tilde_setTimeRand(algo_tilde *x)
{
	double randDoubleFloat;

	randDoubleFloat = rand()/(double)RAND_MAX;

	// note: on this machine, UINT_MAX is twice the size of RAND_MAX
	x->x_t = floor(randDoubleFloat * UINT_MAX); // best to set x->x_t directly, because x->x_tBlockStart grabs that at the beginning of the next block
}

static void algo_tilde_setTimeLoopPoints(algo_tilde *x, t_floatarg t0, t_floatarg t1)
{
	// if any time points are negative, set loop to full range
	// if t1 == t2, set loop to full time range
	if(t0 < 0 || t1 < 0 || t0 == t1)
	{
		x->x_tLoopPoints[0] = 0;
		x->x_tLoopPoints[1] = UINT_MAX;
		return;
	}

	// keep bounded within 0 and UINT_MAX
	t0 = (t0<0)?0:t0;
	t0 = (t0>UINT_MAX)?UINT_MAX:t0;
	t1 = (t1<0)?0:t1;
	t1 = (t1>UINT_MAX)?UINT_MAX:t1;

	// ensure that t1 > t0
	if(t0 > t1)
	{
		uint32_t tmp;
		tmp = t0;
		t0 = t1;
		t1 = tmp;
	}

	x->x_tLoopPoints[0] = t0;
	x->x_tLoopPoints[1] = t1;
}

static void algo_tilde_loopRecord(algo_tilde *x, t_floatarg record)
{
	if(record > 0)
	{
		// first, reset the loop points to MIN/MAX to erase any previously recorded loop
		algo_tilde_setTimeLoopPoints(x, -1, -1);

		if(x->x_timeDirection == forward)
			x->x_tLoopPoints[0] = x->x_t;
		else
			x->x_tLoopPoints[1] = x->x_t;
	}
	else
	{
		if(x->x_timeDirection == forward)
			x->x_tLoopPoints[1] = x->x_t;
		else
			x->x_tLoopPoints[0] = x->x_t;
	}
}

static void algo_tilde_interpSwitch(algo_tilde *x, t_floatarg i)
{
	// keep bounded within 0 and 1
	i = (i<0)?0:i;
	i = (i>1)?1:i;
	x->x_interpSwitch = i;
}

static void algo_tilde_computeSwitch(algo_tilde *x, t_floatarg c)
{
	// keep bounded within 0 and 1
	c = (c<0)?0:c;
	c = (c>1)?1:c;
	x->x_computeSwitch = c;
}

static void algo_tilde_timeDirection(algo_tilde *x, t_floatarg d)
{
	d = (d<-1)?-1:d;
	d = (d>=0)?1:d;
	x->x_timeDirection = d;
}

static void algo_tilde_setAlgo(algo_tilde *x, t_symbol *exprArg)
{
	int i, count;

	// DEBUG: surprisingly, destroying the expression before creating it again causes crashes
	// destroy the old expression first
// 	expr_destroy(x->x_exprExp, &(x->x_exprVars));

	// get the expression string from this argument
	x->x_exprStr = exprArg->s_name;

	// find out how many params are in the expression
	for (i=0, count=0; exprArg->s_name[i]; i++)
	  count += (exprArg->s_name[i] == 'p');

	// update x_numAlgoParams
	x->x_numAlgoParams = count;

	// call _getNumAlgoParams so updated value goes out outlet
	algo_tilde_getNumAlgoParams(x);

	x->x_exprExp = expr_create(x->x_exprStr, strlen(x->x_exprStr), &(x->x_exprVars), exprUserfuncs);

	if(x->x_exprExp == NULL)
		pd_error(x, "%s: syntax error", x->x_objSymbol->s_name);

	// update the linked list of vars
	// DEBUG: only do this if we destroy then create again
// 	for(i=0; i<MAXALGOPARAMS; i++)
// 	{
// 		struct expr_var *v;
//
// 		v = expr_var(&(x->x_exprVars), x->x_paramStrings[i], strlen(x->x_paramStrings[i]));
// 		v->value = x->x_params[i];
// 	}
}

static void algo_tilde_parameters(algo_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;

	for(i=0; i<argc && i<MAXALGOPARAMS; i++)
	{
		uint32_t p;

    // if incoming float parameter is negative, set it to zero
		p = (atom_getfloat(argv+i)<0)?0:atom_getfloat(argv+i);
		p = (p>UINT_MAX)?UINT_MAX:p;
		x->x_params[i] = p;
	}

	// if there weren't enough arguments, fill remaining params with 0
	for(; i<MAXALGOPARAMS; i++)
		x->x_params[i] = 0;

	// now that x_params is filled, update the expr linked list of vars
	for(i=0; i<MAXALGOPARAMS; i++)
	{
		struct expr_var *v;

		v = expr_var(&(x->x_exprVars), x->x_paramStrings[i], strlen(x->x_paramStrings[i]));
		v->value = x->x_params[i];
	}
}

static void algo_tilde_bitDepth(algo_tilde *x, t_floatarg b)
{
	// keep bounded within 1 and MAXBITDEPTH
	b = (b<1)?1:b;
	b = (b>MAXBITDEPTH)?MAXBITDEPTH:b;
	x->x_bitDepth = b;
 	x->x_quantSteps = pow(2.0f, x->x_bitDepth);
}

static void algo_tilde_samplerate(algo_tilde *x, t_floatarg t)
{
	// keep bounded within 1 and MAXSAMPLERATE
	t = (t<1.0)?1.0:t;
	t = (t>MAXSAMPLERATE)?MAXSAMPLERATE:t;
	x->x_samplerate = t;

	x->x_incr = x->x_samplerate/x->x_sr;
}

static void algo_tilde_debug(algo_tilde *x, t_floatarg d)
{
	// keep bounded within 0 and 255
	d = (d<0)?0:d;
	d = (d>255)?255:d;
	x->x_debug = d;
}

static void algo_tilde_savePreset(algo_tilde *x, t_symbol *f)
{
	FILE *filePtr;
  char fileNameBuf[MAXPDSTRING];
	char altFileNameBuf[MAXPDSTRING];
  unsigned char i;
  t_symbol *fileSymbol;

	if(!strcmp(f->s_name, ""))
	{
		sprintf(altFileNameBuf, "preset-%f.txt",  clock_getlogicaltime());
		fileSymbol = gensym(altFileNameBuf);
	}
	else
		fileSymbol = f;

  canvas_makefilename(x->x_canvas, fileSymbol->s_name, fileNameBuf, MAXPDSTRING);

	filePtr = fopen(fileNameBuf, "w");

  if(!filePtr)
  {
    pd_error(x, "%s: failed to create %s", x->x_objSymbol->s_name, fileNameBuf);
    return;
  }

	fprintf(filePtr, "algorithm: %s\n", x->x_exprStr);

  fprintf(filePtr, "parameters: ");

  // TODO: could use x->x_numAlgoParams as the limit instead. this change would require appropriate update to _loadPreset()
  for(i=0; i<MAXALGOPARAMS-1; i++)
    fprintf(filePtr, "%u ", x->x_params[i]);

  fprintf(filePtr, "%u\n", x->x_params[MAXALGOPARAMS-1]);

	// seems like %0.55f is the max number of relevant digits beyond the decimal point for a double precision float, but %0.15f should be good enough and keeps the preset file easy to read
	fprintf(filePtr, "bit-depth: %0.15f\n", x->x_bitDepth);
	fprintf(filePtr, "samplerate: %0.15f\n", x->x_samplerate);
	fprintf(filePtr, "time: %u\n", x->x_t);
	fprintf(filePtr, "loop-points: %u %u\n", x->x_tLoopPoints[0], x->x_tLoopPoints[1]);

  fclose(filePtr);
}

static void algo_tilde_loadPreset(algo_tilde *x, t_symbol *f)
{
	FILE *filePtr;
  char fileNameBuf[MAXPDSTRING], stringIdBuf[MAXPDSTRING], algo[MAXPDSTRING];
  t_float bitDepth, samplerate;
  uint32_t t, params[MAXALGOPARAMS], tLoopPoints[2];
  t_atom *paramAtoms;
  unsigned char i;

  canvas_makefilename(x->x_canvas, f->s_name, fileNameBuf, MAXPDSTRING);

  filePtr = fopen(fileNameBuf, "r");

  if (!filePtr)
  {
    pd_error(x, "%s: failed to open %s", x->x_objSymbol->s_name, fileNameBuf);
    return;
  }

  fscanf(filePtr, "%s", stringIdBuf);
  fscanf(filePtr, "%s", algo);

 	fscanf(filePtr, "%s", stringIdBuf);


  // TODO: could keep reading until encountering "bitDepth" string. otherwise, all preset files must have MAXALGOPARAMS parameters listed
  for(i=0; i<MAXALGOPARAMS; i++)
    fscanf(filePtr, "%u", params+i);

  fscanf(filePtr, "%s", stringIdBuf);
  fscanf(filePtr, "%f", &bitDepth);
  fscanf(filePtr, "%s", stringIdBuf);
  fscanf(filePtr, "%f", &samplerate);
  fscanf(filePtr, "%s", stringIdBuf);
  fscanf(filePtr, "%u", &t);
  fscanf(filePtr, "%s", stringIdBuf);
  fscanf(filePtr, "%u", tLoopPoints);
  fscanf(filePtr, "%u", tLoopPoints+1);

	// load the algo. need to convert the string to a symbol
	algo_tilde_setAlgo(x, gensym(algo));

	// assign the values to the object dataspace

	paramAtoms = (t_atom *)t_getbytes(MAXALGOPARAMS*sizeof(t_atom));

	for(i=0; i<MAXALGOPARAMS; i++)
	{
		uint32_t p;

		p = (params[i]<0)?0:params[i];
		p = (p>UINT_MAX)?UINT_MAX:p;

		SETFLOAT(paramAtoms+i, p);
	}

  if(x->x_presetLoadParams)
  {
  	// x_exprArgs will be updated via this function call, as if the params came in as a "parameters" message
  	algo_tilde_parameters(x, gensym("parameters"), MAXALGOPARAMS, paramAtoms);
  }

	// free local memory
	t_freebytes(paramAtoms, MAXALGOPARAMS * sizeof(t_atom));

	// assign time loop points directly since _setTimeLoopPoints() takes floating point arguments. note that -1 won't work since the data type is uint32_t. could improve this later
	if(tLoopPoints[0] == tLoopPoints[1]) // if t1 ==t2, set loop to full time range
	{
		tLoopPoints[0] = 0;
		tLoopPoints[1] = UINT_MAX;
	}
	else if(tLoopPoints[0] > tLoopPoints[1]) // ensure that t2 > t1
	{
		uint32_t tmp;
		tmp = tLoopPoints[0];
		tLoopPoints[0] = tLoopPoints[1];
		tLoopPoints[1] = tmp;
	}

  if(x->x_presetLoadLoopPoints)
  {
  	x->x_tLoopPoints[0] = tLoopPoints[0];
  	x->x_tLoopPoints[1] = tLoopPoints[1];
  }
	// use existing functions for remaining assignments since they have safety checks

  if(x->x_presetLoadBitDepth)
    algo_tilde_bitDepth(x, bitDepth);

  if(x->x_presetLoadSampleRate)
    algo_tilde_samplerate(x, samplerate);

  if(x->x_presetLoadTime)
    algo_tilde_setTimeIndex(x, t, 0); // set mu to 0

  fclose(filePtr);
}

static void algo_tilde_setPresetLoadParams(algo_tilde *x, t_floatarg flag)
{
	if(flag>0)
    x->x_presetLoadParams = true;
  else
    x->x_presetLoadParams = false;
}

static void algo_tilde_setPresetLoadBitDepth(algo_tilde *x, t_floatarg flag)
{
	if(flag>0)
    x->x_presetLoadBitDepth = true;
  else
    x->x_presetLoadBitDepth = false;
}

static void algo_tilde_setPresetLoadSampleRate(algo_tilde *x, t_floatarg flag)
{
	if(flag>0)
    x->x_presetLoadSampleRate = true;
  else
    x->x_presetLoadSampleRate = false;
}

static void algo_tilde_setPresetLoadTime(algo_tilde *x, t_floatarg flag)
{
	if(flag>0)
    x->x_presetLoadTime = true;
  else
    x->x_presetLoadTime = false;
}

static void algo_tilde_setPresetLoadLoopPoints(algo_tilde *x, t_floatarg flag)
{
	if(flag>0)
    x->x_presetLoadLoopPoints = true;
  else
    x->x_presetLoadLoopPoints = false;
}

static void algo_tilde_initClock(algo_tilde *x)
{
	x->x_startupFlag = 1;
}

static void *algo_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  algo_tilde *x = (algo_tilde *)pd_new(algo_tilde_class);
	t_symbol *exprArg;
	struct expr_var *v;
	int i;

	outlet_new(&x->x_obj, &s_signal);
  x->x_outletTime = outlet_new(&x->x_obj, gensym("list"));
  x->x_outletNumAlgoParams = outlet_new(&x->x_obj, &s_float);
  x->x_outletAlgoSettings = outlet_new(&x->x_obj, gensym("list"));
  x->x_outletWrapBang = outlet_new(&x->x_obj, &s_bang);

	// store the pointer to the symbol containing the object name. Can access it for error and post functions via s->s_name
	x->x_objSymbol = s;
	x->x_startupFlag = 0; // keep track of whether the object creation process has completed
  x->x_clock = clock_new(x, (t_method)algo_tilde_initClock);

	x->x_sr = DEFAULTSAMPLERATE;
	x->x_n = 64.0f;

	x->x_samplerate = DEFAULTSAMPLERATE;

	// this function handles calculation of x_quantSteps
	algo_tilde_bitDepth(x, 8.0f);

	x->x_interpSwitch = 1;
	x->x_computeSwitch = 1;
	x->x_timeDirection = forward;
  x->x_presetLoadParams = true;
  x->x_presetLoadBitDepth = true;
  x->x_presetLoadSampleRate = true;
  x->x_presetLoadTime = true;
  x->x_presetLoadLoopPoints = true;
	x->x_debug = 0;

	x->x_t = 0;
	x->x_tBlockStart = 0;
	x->x_tBlockEnd = 0;
	x->x_tLoopPoints[0] = 0;
	x->x_tLoopPoints[1] = UINT_MAX; // loop points default to full time range
	x->x_mu = 0.0f;
	x->x_incr = 0.0f;
	x->x_sampIdx = 0.0f;

	// this recreates results of 8kHz sampling rate when running at 44.1kHz
	algo_tilde_samplerate(x, 8000.0f);

	x->x_signalBuffer = (double *)t_getbytes((x->x_n*4+EXTRAPOINTS) * sizeof(double));

	for(i=0; i<x->x_n*4+EXTRAPOINTS; i++)
		x->x_signalBuffer[i] = 0.0;

	// copy the global const array contents to arrays within our object dataspace
	memcpy(x->x_array36364689, array36364689, sizeof(array36364689));
	memcpy(x->x_paramStrings, paramStrings, sizeof(paramStrings));

	// parse creation arguments
	switch(argc)
	{
		case 0:
			exprArg = gensym("t*p0");
			algo_tilde_setAlgo(x, exprArg);
			algo_tilde_parameters(x, gensym("parameters"), argc, argv);
			break;

		case 1:
			exprArg = atom_getsymbol(argv);
			algo_tilde_setAlgo(x, exprArg);
			algo_tilde_parameters(x, gensym("parameters"), 0, argv);
			break;

		default:
			exprArg = atom_getsymbol(argv);
			algo_tilde_setAlgo(x, exprArg);
			algo_tilde_parameters(x, gensym("parameters"), argc-1, argv+1);
			break;
	}

	// now that expr_create() has been called, initialize "t" in exprVars
	v = expr_var(&(x->x_exprVars), "t", strlen("t"));
	v->value = x->x_t;

	// seed randomness via current time
	srand(clock_getlogicaltime());

  x->x_canvas = canvas_getcurrent();

	post("%s version %s", x->x_objSymbol->s_name, ALGOTILDEVERSION);

	clock_delay(x->x_clock, 0); // wait 0ms to give a control cycle for expr_create() to be called

  return(x);
}

static t_int *algo_tilde_perform(t_int *w)
{
  uint32_t i, n, m, hop;

  algo_tilde *x = (algo_tilde *)(w[1]);
  t_sample *out = (t_float *)(w[2]);
  n = w[3];

	// the rough number of algo samples to compute (m) is the blocksize (n) scaled by the increment. we take the ceiling to make sure we generate more than needed
	m = ceil(n*x->x_incr);

	// note the current time index at the start of the block
	x->x_tBlockStart = x->x_t;

	if(x->x_computeSwitch)
	{
		double sampIdxBlockEnd;
		unsigned char wrapFlag;
		uint32_t wrapIdx;

		// clear the wrap flag for this block
		wrapFlag = 0;
		wrapIdx = UINT_MAX;

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
			double thisSampleDouble;
			uint32_t thisSample;
			struct expr_var *v;

			// only update t in arg list and evaluate expression if one exists
			if(x->x_exprExp)
			{
				// update t
				v = expr_var(&(x->x_exprVars), "t", strlen("t"));
				v->value = x->x_t;

				// perform the algorithm via expr_eval() in double precision. we store the result in unsigned long int, which truncates any fractional part. note that double has the precision to reference all integers in 32-bit UINT data type, so we lose nothing.
				thisSample = expr_eval(x->x_exprExp);
			}
			else
				thisSample = 0;

			// convert the uint32_t sample into a double precision float sample
			thisSampleDouble = thisSample/x->x_quantSteps;
			thisSample = floor(thisSampleDouble);
			thisSampleDouble = thisSampleDouble - thisSample;
			thisSampleDouble = thisSampleDouble*2.0-1.0;
			x->x_signalBuffer[i] = thisSampleDouble;

			if(x->x_debug)
			{
				post("%i: [%0.6f], t: %lu", i, x->x_signalBuffer[i], x->x_t);
				// don't decrement debug until end of perform routine
			}

			// advance the time variable t, wrapping via x_tLoopPoints if needed
			// turn the wrap flag on if this happens
			switch(x->x_timeDirection)
			{
				case forward:
					x->x_t++;
					// wrap if we hit the end point of the loop
					// this means that x_t will never be assigned x_tLoopPoints[1]
					if(x->x_t >= x->x_tLoopPoints[1])
					{
						x->x_t = x->x_tLoopPoints[0];
						// note the sample where the wrapping occurred
						wrapIdx = i;
					}
					break;
				case backward:
					x->x_t--;
					if(x->x_t <= x->x_tLoopPoints[0])
					{
						x->x_t = x->x_tLoopPoints[1];
						wrapIdx = i;
					}
					break;
				default:
					x->x_t++;
					if(x->x_t >= x->x_tLoopPoints[1])
					{
						x->x_t = x->x_tLoopPoints[0];
						wrapIdx = i;
					}
					break;
			}
		}

	////////////////////////////////
	//	DO INTERPOLATION
	//	note that this is not affected by time direction
	//	once x_signalBuffer is filled based on time direction, the job here
	//	is just to pull interpolated samples from that buffer
	//
		// to start the block, sampIdx should be 1.mu from last time
		x->x_sampIdx = (x->x_sampIdx - floor(x->x_sampIdx)) + 1.0;

		if(x->x_debug)
			post("pre-output sampIdx: %f", x->x_sampIdx);

		// interpolate and output
		for(i=0; i<n; i++)
		{
			uint32_t j;

			if(x->x_interpSwitch)
				x->x_mu = x->x_sampIdx - floor(x->x_sampIdx);
			else
				x->x_mu = 0.0;

			j = floor(x->x_sampIdx);

			out[i] = algo_tilde_cubicInterpolate(x->x_signalBuffer[j-1], x->x_signalBuffer[j], x->x_signalBuffer[j+1], x->x_signalBuffer[j+2], x->x_mu);

			if(x->x_debug)
			{
				post("%i: [%0.6f, %0.6f, %0.6f, %0.6f][%0.6f, %0.6f], mu: %f, sampIdx: %f, j: %i, jInterp: %f, [%0.6f]", i, x->x_signalBuffer[j-1], x->x_signalBuffer[j], x->x_signalBuffer[j+1], x->x_signalBuffer[j+2], x->x_signalBuffer[j+3], x->x_signalBuffer[j+4], x->x_mu, x->x_sampIdx, j, j+x->x_mu, out[i]);
				// don't decrement debug until end of perform routine
			}

			// before incrementing x_sampIdx, turn on the wrap flag if we actually got to the sample where the wrapping occurred. need to subtract 2 because x_sampIdx starts at 1.mu. otherwise it's possible to get two wrap bangs in a row in borderline cases
			if(x->x_sampIdx-2 > wrapIdx)
				wrapFlag = 1;

			sampIdxBlockEnd = x->x_sampIdx;
			x->x_sampIdx += x->x_incr;
		}
	//
	//
	//
	//	END INTERPOLATION
	////////////////////////////////

		if(x->x_debug)
		{
			post("sampIdxBlockEnd: %f", sampIdxBlockEnd);
			post("post-output sampIdx: %f", x->x_sampIdx);
		}

		// at this point, if we take floor of x_sampIdx - 1, we know how many samples we have moved through in the interpolation loop of n samples above. this is how much we should move forward in time, and this is also the sample in the signal buffer we should move to the head (index 0) of the next signal block.
		hop = floor(x->x_sampIdx - 1.0);

		// store the final sample of the algo signal block to index 0 of the signal buffer for next time. since we generated extra samples past the m samples we actually want to hear, we need to rewind the time index. the time index for next block should be hop samples past the time index from the start of this block. remember to wrap at UINT_MAX as our maximum time index.
		x->x_signalBuffer[0] = x->x_signalBuffer[hop];

		// rewind x_t to x_tBlockStart to prepare for wrap check below
		x->x_t = x->x_tBlockStart;

		if(x->x_timeDirection==forward)
		{
			uint32_t tmpHop;

			tmpHop = hop;

			// move forward by hop samples, wrapping if needed
			while(tmpHop--)
			{
				// if we hit the loop end, jump back to loop start
				if(x->x_t >= x->x_tLoopPoints[1])
					x->x_t = x->x_tLoopPoints[0];
				else
					x->x_t++;
			}
		}
		else if(x->x_timeDirection==backward)
		{
			uint32_t tmpHop;

			tmpHop = hop;

			// move backward by hop samples, wrapping if needed
			while(tmpHop--)
			{
				// if we hit the loop start, jump to loop end
				if(x->x_t <= x->x_tLoopPoints[0])
					x->x_t = x->x_tLoopPoints[1];
				else
					x->x_t--;
			}
		}

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

		// bang if time has wrapped, indicating pattern reset
		if(wrapFlag)
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

static void algo_tilde_dsp(algo_tilde *x, t_signal **sp)
{
	dsp_add(
		algo_tilde_perform,
		3,
		x,
		sp[0]->s_vec,
		sp[0]->s_n
	);

	if(x->x_n != sp[0]->s_n)
	{
		x->x_signalBuffer = (double *)t_resizebytes(x->x_signalBuffer, (x->x_n*4+EXTRAPOINTS) * sizeof(double), (sp[0]->s_n*4+EXTRAPOINTS) * sizeof(double));

		x->x_n = sp[0]->s_n;
	}

	if(x->x_sr != sp[0]->s_sr)
	{
		x->x_sr = sp[0]->s_sr;
		// update x->x_incr via the _samplerate() method
		algo_tilde_samplerate(x, x->x_samplerate);
	}
};


static void algo_tilde_free(algo_tilde *x)
{
	// free the signalBuffer memory
	t_freebytes(x->x_signalBuffer,  (x->x_n*4+EXTRAPOINTS)*sizeof(double));

	// free the expression
	expr_destroy(x->x_exprExp, &(x->x_exprVars));

	// free the clock
	clock_free(x->x_clock);
};

void algo_tilde_setup(void)
{
  algo_tilde_class =
  class_new(
  	gensym("algo~"),
  	(t_newmethod)algo_tilde_new,
  	(t_method)algo_tilde_free,
    sizeof(algo_tilde),
    CLASS_DEFAULT,
    A_GIMME,
	  0
  );

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setAlgo,
		gensym("algo"),
		A_DEFSYMBOL,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_print,
		gensym("print"),
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_getAlgoSettings,
		gensym("getAlgoSettings"),
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_getTimeIndex,
		gensym("getTimeIndex"),
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_getNumAlgoParams,
		gensym("getNumAlgoParams"),
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setTimeIndex,
		gensym("setTimeIndex"),
		A_DEFFLOAT,
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setTimeRand,
		gensym("setTimeRand"),
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setTimeLoopPoints,
		gensym("setTimeLoopPoints"),
		A_DEFFLOAT,
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_loopRecord,
		gensym("loopRecord"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_interpSwitch,
		gensym("interpolate"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_computeSwitch,
		gensym("compute"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_timeDirection,
		gensym("timeDirection"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_parameters,
		gensym("parameters"),
		A_GIMME,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_bitDepth,
		gensym("bitDepth"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_samplerate,
		gensym("sampleRate"),
		A_DEFFLOAT,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_debug,
		gensym("debug"),
		A_DEFFLOAT,
		0
	);

	// it looks like changing the number of class_addmethod() function calls here in _setup() changes the output of algorithms involving the 36364689 char array as well. even their ORDER listed here changes it. additional code elsewhere (like the _loadPreset function definition) don't seem to affect this. nor does additional memory (even large amounts!) declared in the object's data struct. it is only within the _setup() function. it would be excellent to figure out how this works, and perhaps find a way to work toward a modulo value other than 256 for the index into the char array (which invokes undefined behavior since the max index should be 7). the two issues may be interrelated
	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_loadPreset,
		gensym("load"),
		A_DEFSYMBOL,
		0
	);

	class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_savePreset,
		gensym("save"),
		A_DEFSYMBOL,
		0
	);

  class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setPresetLoadParams,
		gensym("loadParams"),
		A_DEFFLOAT,
		0
	);

  class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setPresetLoadBitDepth,
		gensym("loadBitDepth"),
		A_DEFFLOAT,
		0
	);

  class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setPresetLoadSampleRate,
		gensym("loadSampleRate"),
		A_DEFFLOAT,
		0
	);

  class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setPresetLoadTime,
		gensym("loadTime"),
		A_DEFFLOAT,
		0
	);

  class_addmethod(
		algo_tilde_class,
		(t_method)algo_tilde_setPresetLoadLoopPoints,
		gensym("loadLoopPoints"),
		A_DEFFLOAT,
		0
	);

  class_addmethod(
  	algo_tilde_class,
  	(t_method)algo_tilde_dsp,
  	gensym("dsp"),
  	0
  );
}
