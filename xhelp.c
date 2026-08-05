#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int showtext=0;

#define MAXBUF 100

int main(int argc, char *argv[]) {
    FILE *fp;
    char buf[MAXBUF];
    int print=0;

    if ((fp=fopen("./help.txt","r"))==NULL) {
	fprintf(stderr,"could not open helpfile\n");
	exit(2);
    }
    if (argc>1)  showtext++;
    
    while((fgets(buf, MAXBUF, fp))!=NULL) {
	if (strstr(buf,"T:")) {
	    print=0;
	    if (!showtext) {
		printf("\t%s", buf);
	    } else if (strstr(buf,argv[1])) {
		// printf("\t%s", buf);
		print++;
	    }
	} else {
	    if (print) printf("\t%s", buf);
	}
	// FIXME, print error if nothing matched
    }
}


/*

U: Type "help <topic>" to see more details
T: options 
[-g] ; use gnuplot rather than pdplot
[-r <rawfile.raw>] ; load in rawfile before starting
[-?] // show usage message
T: rawfile (ci se ls di)
ls		 // list all rawfiles in the current directory
ci <rawfile.raw> // load in a new analysis
di               // display loaded variables
se               // list all loaded analyses
se <n>           // switch to the nth analysis
T: plotting (gr gs xl yl logx logy logf)
examples:
gr v1,v2  // plot waveform v1,v2 on a new graph
gs v3	  // add waveform v3 to an existing graph
gr db(v1) // plot 20*log(v1) on a new graph
gr sqrt(v1) xl 0,10 yl 0,10 // plot a function with x,y limits
gr v1 logx // plot with log x scale
gr v1 logf // plot with log frequency scale
gr v1,v2;v3,v4  // plot two groups of variable on separate axes
gr v1,v2 yl 0,10; v3,v4 yl -1,4  // separate yl commands per axis
T: variables (pwl constants complex)
examples:
gr v1-v2 // plot the difference of two voltages
vdiff = v1-v2 // perform PWL math and save in symbol table
gr vdiff // plot the saved difference voltage
x=3+2i   // specify a complex number using "i"
p1={0,0; 1,4; 3,4; 4,3} // create a new piece wise linear variable
p1(1.1)  // return the y value of p1 at x=1.1
T: functions 
for p1,p2,p3 all piece-wise-linear functions, d=real
p3=avg(p1,p2)       // p3(k) = (p1(k)+p2(k))/2
d3=avg(d1,d2)       // d3 = (d1+d2)/2
p3=avg(d1,p1)       // p3(k) = (p1(k)+d1)/2
p3=avg(p1,d1)       // p3(k) = (p1(k)+d1)/2
d=avg(p)            // compute average value of a single PWL
p=db(p)             // return 20*log10(mag(p))
d=db(d)             // return 20*log10(mag(d))
p2=dt(p1)           // returns (p1(t)-p1(t+DT))/DT, DT is global
p2=exp(p1)          // p2(k) = e^p1(k)
p2=integral(p1)     // return the running integral of p1*DT
p2=ln(p1)           // p2(k) = ln(p1(k))
p2=log10(p1)        // p2(k) = ln(p1(k))/ln(10)
p2=log(p1)          // p2(k) = ln(p1(k))/ln(10)
p2=lpf(p1,tau) 	    // returns a PWL of p1 filter by single pole tau
p2=mag(p1)          // p2(k) = sqrt((p1(k).re)^2+(p1(k).im)^2)
p3=mod(p1,p2)       // p3.re = fmod(p1.re,p2.re), p3.im=0.0
p3=max(p1,p2)       // p3 = (p1(k).re>p2(k).re)?p1(k):p2(k)
p3=min(p1,p2)       // p3 = (p1(k).re<p2(k).re)?p1(k):p2(k)
d=max(p1)	    // return maximum y val in p1
d=min(p1)	    // return minimum y val in p1
p2=pha(p1)          // p2(k) = atan2(p1(k).im, p1(k).re)
d2=pha(d)           // d2 = atan2(d.im, d.re)
p3=pow(p1,p2)       // p3(k) = p1(k)^p2(k)
p3=pow(p1,d2)       // p3(k) = p1(k)^d
d=pow(d1,d2)        // d = d1^d2
p3=p1^p2            // alternative ways of computing pwl^pwl 
d3=d1^d2	    // alternative way of computing scalar powers
d3=p1^d1	    // alternative way to raise pwl to a power
p2=re(p1)           // p2(k) = p1(k).re
d2=re(d)            // d2 = d.re
p2=im(p1)           // p2(k) = imaginary part of p1(k)
d2=im(d)            // d2 = d.im
gr versus(p1,p2)    // plot p1 versus v2
gr versus(p1,mod(time, 1n))         // plot a 1ns eye diagram of p1
gr versus(p1,mod(time+200p, 1n))  // plot a shifted eye diagram
d2=delay(d1,1n)  // return a waveform shifted in time by 1ns
d2=cos(d1)	 // compute cos(d1)
d2=sin(d1)	 // compute cos(d1)
d2=sqrt(d1)	 // compute sqrt(d1)
T: measurement (xcross xcrossp xcrossn)
t=xcross(p1,n) 	// return the time of nth zero crossing of p1
t=xcrossp(p1,p2) // return time of nth positive going zero crossing of p1
t=xcrossn(p1,p2) // return time of nth negative going zero crossing of p1
T: control (pause exit)
pause(200)  // pause for 200 seconds unless a keystroke is typed
exit,quit,bye // all equivalent ways to exit the interpreter
*/
