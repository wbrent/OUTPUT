#include "outputAlgo~.h"

static unsigned long int outputAlgo_tilde_getSample(outputAlgo_tilde *x)
{
    unsigned int t, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9;
    unsigned long int thisSample;

	t = x->x_t;	
	// avoid t==0, which would cause division or modulus by zero in some algos
	t=(t==0)?1:t;
	
	// grab the params from the array before the for loop to simplify algos
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
	
	// NOTE: for all divisions, cast divisor (float), then cast the result as (int)
	switch(x->x_algoChoice)
	{
		case 0:
			thisSample = t*((t>>p0|t>>p1)&p2&t>>p3); // IDENT: 0, 36, 38, 83
			break;
		case 1:
			thisSample = t*(p0&t>>p1)|((t*p2)*(t>>p3)*(t<<p4));
			break;
		case 2:
			thisSample = ((t*p0)*(t>>p1)|(t<<p2));
			break;
		case 3:
			thisSample = t*((t>>p0)*(t>>p1))|((t>>p2)&(p3&t>>p4));
			break;
		case 4:
			thisSample = (t*p0&(t>>p1))|((t*p2)&(t*p3>>p4));
			break;
		case 5:
			thisSample = t&t>>p0;
			break;
		case 6:
			thisSample = (t*p0)&t>>p1;
			break;
		case 7:
			thisSample = t&p0&t>>p1;
			break;
		case 8:
			thisSample = t*(p0&t>>p1);
			break;
		case 9:
			thisSample = t*(t>>p0|t>>p1)&p2;
			break;
		case 10:
			thisSample = (t&t%p0)-(t*p1&t>>p2&t>>p3);
			break;
		case 11:
			thisSample = ((int)(t/((float)1e7*t*t+t)))%p0|t>>p1|t>>p2|t%p3+(t>>p4)|t;
			break;
		case 12:
			thisSample = ((((t%p0)+t)|t)<<p1)+(t&(t>>p2))+(t<<p3);
			break;
		case 13:
			thisSample = (t*p0&(t>>p1))|(t*p2&(t*p3>>p4));
			break;
		case 14:
			thisSample = (t*p0&t>>p1)|(t*p2&t>>p3);
			break;
		case 15:
			thisSample = (t*p0&t>>p1)|(t*p2&t>>p3)|(t*p4&((int)(t/(float)p5)));
			break;
		case 16:
			thisSample = t*(0xCA98>>(t>>p1&p2)&p3)|t>>p4;
			break;
		case 17:
			thisSample = ((t*p0&t>>p1)|(t*p2&t>>p3)|(t*p4&t/p5))-p6;
			break;
		case 18:
			thisSample = ((t*(t>>p0)&(p1*(int)(t/(float)100))&(p2*(int)(t/(float)100)))&(t*(t>>p3)&(t*(int)(p4/(float)100))&(t*(int)(p5/(float)100))))+((t*(t>>p6)&(t*(int)(p7/(float)100))&(t*(int)(p8/(float)100)))-(t*(t>>p9)&(t*(int)(302/(float)100))&(t*(int)(298/(float)100))));
			break;
		case 19:
			thisSample = (((int)(t/(float)(2*(p0&(0x234568a0>>(t>>p1&p2)))))))|((int)(t/(float)(2>>(t>>p3)))^t>>p4)+((int)(t/(float)(16&t&p5)));
			break;
		case 20:
			thisSample = ((t*(int)(p0/(float)(t&p1+p2))&t>>p3)&((t*p4)&t>>p5))|(t>>p6&p7);
			break;
		case 21:
			thisSample = ((t*(p0|(t&p1+p2))&t>>p3)&((t*p4)&t>>p5))|(t>>p6&p7);
			break;
		case 22:
			thisSample = ((((int)((((t>>p0)%p1)*t)/(float)(t%p2))|(int)(t/(float)((p3*(((t>>p4)%p5)+p6)))))^((t>>p7)%p8))-p9);
			break;
		case 23:
			thisSample = t>>p0|(int)((t&(t>>p1))/(float)(t>>(p2-(t>>p3))& t >> (p4-(t>>p5))));
			break;
		case 24:
			thisSample = (((((t%p0) + (t%p1)) | t) | (t>>p2 & t>>p3))%p4) | t>>p5;
			break;
		case 25:
			thisSample = t&(t>>p0)>>p1&t>>p2;
			break;
		case 26:
			thisSample = (((int)((((t>>p0)^(t>>p1)-p2)%p3*t)/(float)p4)|t>>p5)&p6);
			break;
		case 27:
			thisSample = ((int)((t*("36364689"[t>>p0&p1]&p2))/(float)p3)&p4);
			break;
		case 28:
			thisSample = (t>>p0)|(t>>p1)|(int)(((t%p2)*(t>>p3)|(0x15483113)-(t>>p4))/(float)((t>>p5)^(t|(t>>p6))));
			break;
		case 29:
			thisSample = ((int)(t*p0/(float)p1)|t*p2+(t<<p3));
			break;
		case 30:
			thisSample = (t<65536)?(((p0*t*(t>>p1)&(t-p2))|(t>>p3)-1)%p4):(((t%98304)>65536)?(((p5*t*(t*t>>p6)&(t-1))|(t>>p7)-1)%p8|(t>>4)):(((13*t*(2*t>>p9)&(t-1))|(t>>8)-1)%32|(t>>4)));
			break;
		case 31:
			thisSample = t>>p0|((t>>p1)%p2)|((t>>p3)%p4)|(t*t%p5)|(t*t%p6)|(t>>p7)*(t|t>>p8);
			break;
		case 32:
			thisSample = (t>>p0^(t&p1))|(t+(t^t>>p2)-t*((t%p3?p4:p5)&t>>p6)^(t<<1&(t&p7?t>>p8:t>>p9)));
			break;
		case 33:
			thisSample = (((int)(t/(float)p0)*(p1&(0x234568a0>>(t>>p2&p3))))|((int)(t/(float)p4)>>(t>>p5)^t>>p6))+((int)(t/(float)p7)&t&p8);
			break;
		case 34:
			thisSample = (t<65536)?(((p0*t*(t>>p1)&(t-p2))|(t>>p3)-p4)%p5):(((t%98304)>65536)?(((17*t*(p6*t>>8)&(t-1))|(t>>6)-1)%p7|(t>>4)):(((15*t*(2*t>>p8)&(t-1))|(t>>8)-1)%p9|(t>>4)));
			break;
		case 35:
			thisSample = ((t>>p0)*(p1&(0x8898a989>>(t>>p2&p3)))&p4)+((((t>>p5|(t>>p6)|t>>p7)*p8+4*(((t>>2)&t>>p9)|t>>8))&255)>>1);
			break;
		case 36:
			thisSample = t*(((t>>p0)|(t>>p1))&(p2&(t>>p3))); // IDENT: 0, 36, 38, 83
			break;
		case 37:
			thisSample = (t*(t>>p0|t>>p1))>>(t>>p2);
			break;
		case 38:
			thisSample = t*(((t>>p0)|(t>>p1))&(p2&(t>>p3))); // IDENT: 0, 36, 38, 83
			break;
		case 39:
			thisSample = t*(((t>>p0)&(t>>p1))&(p2&(t>>p3)));
		case 40:
			thisSample = t*(t>>p0*((t>>p1)|(t>>p2))&(p3|(t>>p4)*p5>>t|(t>>p6)));
			break;
		case 41:
			thisSample = ((int)(t*t/(float)p0)&(t>>((int)(t/(float)p1)%p2)))^t%p3*(0xC0D3DE4D69>>(t>>p4&p5)&t%p6)*t>>p7;
			break;
		case 42:
			thisSample = t*(t>>((t>>p0)|(t>>p1))&(p2&(t>>p3)));
			break;
		case 43:
			thisSample = (t>>p0|t|t>>(t>>p1))*p2+((t>>p3)&p4);
			break;
		case 44:
			thisSample = ((t%p0-(t>>p1|t*p2|t%p3)-t)>>p4)|(int)((((t>>p5)&(t<<p6)*1663)|(t>>p7)%1544)/(float)(t%p8|t%p9));
			break;
		case 45:
			thisSample = (t|(t>>p0|t>>p1))*t&(t>>p2|t>>p3);
			break;
		case 46:
			thisSample = (t*p0&(t>>p1))|(t*p2&(t*p3>>p4));
			break;
		case 47:
			thisSample = (t>>p0|t|t>>p1)*p2+p3*((t&t>>p4)|t>>p5);
			break;
		case 48:
			thisSample = ((t&p0)?((t*(t^t%p1)|(t>>p2))>>p3):(t>>p4)|((t&p5)?t<<p6:t));
			break;
		case 49:
			thisSample = ((t*(t>>p0|t>>p1)&p2&t>>p3))^((t&t>>p4)|t>>p5);
			break;
		case 50:
			thisSample = (t*p0&t>>p1)|(t*p2&t>>p3);
			break;
		case 51:
			thisSample = ((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t));
			break;
		case 52:
			thisSample = (((int)(t/(float)p0)*(p1&(0x234568a0>>(t>>p2&p3))))|((int)(t/(float)p4)>>(t>>p5)^t>>p6))+((int)(t/(float)p7)&t&p8);
			break;
		case 53:
			thisSample = (t&t%p0)-(t*p1&t>>p2&t>>p3);
			break;
		case 54:
			thisSample = t>>p0|(t&(int)((t>>p1)/(float)(t>>(p2-(t>>p3))&-t>>(p4-(t>>p5)))));
			break;
		case 55:
			thisSample = ((int)((t*("36364689"[t>>p0&p1]&p2))/(float)p3)&128)+(((int)((((t>>p4)^(t>>p5)-p6)%p7*t)/(float)p8)|t>>p9)&127);
			break;
		case 56:
			thisSample = ((t*p0&t>>p1)|(t*p2&t>>p3)|(t*p4&(int)(t/(float)p5)))-p6;
			break;
		case 57:
			thisSample = ((t*(t>>p0)&((int)(p1*t/(float)100))&((int)(p2*t/(float)100)))&(t*(t>>p3)&((int)(t*p4/(float)100))&((int)(t*p5/(float)100))))+((t*(t>>p6)&((int)(t*p7/(float)100))&((int)(t*p8/(float)100)))-(t*(t>>p9)&((int)(t*302/(float)100))&((int)(t*298/(float)100))));
			break;
		case 58:
			thisSample = ((int)(t*(("36364689"[t>>p0&p1]&p2))/(float)p3)&p4)+(((int)((((t>>p5)^(t>>p6)-p7)%p8*t)/(float)4)|t>>p9)&127);
			break;
		case 59:
			thisSample = t*(t^t+(t>>p0|p1)^(t-p2^t)>>p3);
			break;
		case 60:
			thisSample = ((1-(((t+p0)>>((t>>p1)&((t>>p2))))&(t>>p3&-2)))*2)*(((t>>p4)^((t+((t>>p5)&p6))>>p7))&p8)*32+p9;
			break;
		case 61:
			thisSample = ((t>>p0%p1)+p2)*p3*t>>p4*t>>p5;
			break;
		case 62:
			thisSample = t*(((t>>p0)&p1)|(((t>>p2)&p3)^((t>>p4)&p5&(t>>p6))));
			break;
		case 63:
			thisSample = (((int)(t*t/(float)p0))&(t>>((int)(t/(float)p1)%p2)))^t%p3*(0xC0D3DE4D69>>(t>>p4&p5)&t%p6)*t>>p7;
			break;
		case 64:
			thisSample = t|t%p0|t%p1;
			break;
		case 65:
			thisSample = t>>p0&p1?t>>p2:-t>>p3;
			break;
		case 66:
			thisSample = t*(t>>p0|t>>p1)&p2;
			break;
		case 67:
			thisSample = (t&t>>p0)*(t>>p1|t>>p2);
			break;
		case 68:
			thisSample = (t*p0&t>>p1)|(t*p2&t>>p3);
			break;
		case 69:
			thisSample = (t*(t>>p0|t>>p1))>>(t>>p2);
			break;
		case 70:
			thisSample = (t*p0&(t>>p1))|(t*p2&(t*p3>>p4));
			break;
		case 71:
			thisSample = (t>>p0|t%p1)&(t>>p2|t%p3);
			break;
		case 72:
			thisSample = (t*((t>>p0|t>>p1)&p2))&p3;
			break;
		case 73:
			thisSample = (t&t%p0)-(t*p1&t>>p2&t>>p3);
			break;
		case 74:
			thisSample = (t&t>>p0)*(t>>p1|t>>p2)^t>>p3;
			break;
		case 75:
			thisSample = t*(((t>>p0)^((t>>p1)-p2)^p3)%p4);
			break;
		case 76:
			thisSample = t*(0xCA98>>(t>>p0&p1)&p2)|t>>p3;
			break;
		case 77:
			thisSample = ((int)(t/(float)p0))>>(t>>p1)*(int)(t/(float)((t>>p2&p3)+p4));
			break;
		case 78:
			thisSample = (~((int)(t/(float)p0))|(t*p1))^((t*p2&(t>>p3))&t);
			break;
		case 79:
			thisSample = (t|(t>>p0|t>>p1))*t&(t>>p2|t>>p3);
			break;
		case 80:
			thisSample = ((t>>p0%p1)+p2)*p3*t>>p4*t>>p5;
			break;
		case 81:
			thisSample = ((t&p0)?((t*(t^t%p1)|(t>>p2))>>p3):(t>>p4)|((t&p5)?t<<p6:t));
			break;
		case 82:
			thisSample = t*(((t>>p0)|(t>>p1))&(p2&(t>>p3)));
			break;
		case 83:
			thisSample = t*(((t>>p0)|(t>>p1))&(p2&(t>>p3))); // IDENT: 0, 36, 38, 83
			break;
		case 84:
			thisSample = t*(t^t+(t>>p0|p1)^(t-p2^t)>>p3);
			break;
		case 85:
			thisSample = t*(((t>>p0)&(t>>p1))&(p2&(t>>p3)));
			break;
		case 86:
			thisSample = (t>>p0|t|t>>p1)*p2+p3*((t&t>>p4)|t>>p5);
			break;
		case 87:
			thisSample = ((t*p0&t>>p1)|(t*p2&t>>p3)|(t*p4&(int)(t/(float)p5)))-p6;
			break;
		case 88:
			thisSample = t*(t>>((t>>p0)|(t>>p1))&(p2&(t>>p3)));
			break;
		case 89:
			thisSample = (t>>p0|t|t>>(t>>p1))*p2+((t>>p3)&p4);
			break;
		case 90:
			thisSample = (t>>p0)*(0xbad2dea1>>(t>>p1)&p2)|t>>p3;
			break;
		case 91:
			thisSample = (t>>p0)*(p1&(0x8898a989>>(t>>p2&p3)));
			break;
		case 92:
			thisSample = (t>>(t&p0))|(t<<(t&p1))|(t>>p2)|(t<<p3);
			break;
		case 93:
			thisSample = (t>>p0|t%p1)&(t>>p2|t%p3)&(t>>p4|t%p5);
			break;
		case 94:
			thisSample = (t>>p0|t<<p1)+(t>>p2|t<<p3|t>>p4)|t>>p5|t<<p6;
			break;
		case 95:
			thisSample = t+((t&t)^t>>p0)-t*((t>>p1)&(t%p2?p3:p4)&t>>p5);
			break;
		case 96:
			thisSample = ((t*(t>>p0|t>>p1)&p2&t>>p3))^((t&t>>p4)|t>>p5);
			break;
		case 97:
			thisSample = t*(((t>>p0)^((t>>p1)-p2)^p3)%p4);
			break;
		case 98:
			thisSample = (t>>p0)|(t<<p1)|((t&p2)^p3)|((t-p4)>>p5);
			break;
		case 99:
			thisSample = t*(int)(t/(float)p0)-t*(int)(t/(float)p1)+t*(t>>p2|t>>p3|(t<<p4&t>>p5));
			break;
		case 100:
			thisSample = ((t>>p0&t)-(t>>p1)+(t>>p2&t))+(t*((t>>p3)&p4));
			break;
		case 101:
			thisSample = (t*((p0+(p1^(t>>p2&p3)))*(p4+(p5&t>>p6))))>>(t>>p7&p8);
			break;
		case 102:
			thisSample = t>>p0|(t&(int)((t>>p1)/(float)(t>>(p2-(t>>p3))&-t>>(p4-(t>>p5)))));
			break;
		case 103:
			thisSample = ((t>>p0)*(p1&(0x8898a989>>(t>>p2&30)))&255)+((((t>>p3|(t>>p4)|t>>8)*p5+4*(((t>>p6)&t>>p7)|t>>p8))&255)>>p9);
			break;
		case 104:
			thisSample = t*((0xbadbea75>>((t>>p0)&p1)&p2)*p3*(0x5afe5>>((t>>p4)&p5)&p6));
			break;
		case 105:
			thisSample = t*(((t>>p0)&p1)|(((t>>p2)&p3)^((t>>p4)&p5&(t>>p6))));
			break;
		case 106:
			thisSample = (~t>>p0)*((p1&t*(p2&t>>p3))<(p4&t*(p5+(p6&t>>p7))));
			break;
		case 107:
			thisSample = (t+(t>>p0)|(t>>p1))+(t>>p2)|((t>>p3)|(t>>p4)|(t>>p5));
			break;
		case 108:
			thisSample = t*(t>>p0*((t>>p1)|(t>>p2))&(p3|(t>>p4)*p5>>t|(t>>p6)));
			break;
		case 109:
			thisSample = (t>>p0)|(t%p1)|(((t%p2)|(t>>p3))&((t>>p4)|(t*t%p5)));
			break;
		case 110:
			thisSample = (((t&((t>>p0)))+(t|(t>>p1)))&(t>>p2))|((t>>p3)&(t*(t>>p4)));
			break;
		case 111:
			thisSample = (int)(((t%p0)*(t>>p1)|(0x15483113)-(t>>p2))/(float)(t>>p3))^(t|(t>>p4));
			break;
		case 112:
			thisSample = t*(t>>((t&p0)?((int)((t*t)/(float)p1)):((int)(t/(float)p2))))|(t<<((int)(t/(float)p3)))|(t>>p4);
			break;
// 		case 113:
// 			// this is causing crashes with large param values
// 			// suspecting a modulus by zero, which could come up with some of the bitshifting
// 			thisSample = (((((t*((t>>p0|t>>p1)&p2))&(int)(p3/(float)p4))*p5)%(p6<<p7))<<p8)%p9<<4;
// 			break;
		default:
			thisSample = 0;
			break;					
	}
	
	return(thisSample);
}