/*
 * teamath.cpp
 *
 *  Created on: Apr 22, 2019
 *      Author: tea
 */

#include<gsl/gsl_randist.h>
#include<gsl/gsl_cdf.h>
#include"teamath.h"

double chi2pdf(double x, double V)
{
	return gsl_ran_chisq_pdf(x, V);
}
double chi2cdf(double x, double V)
{
	return gsl_cdf_chisq_P(x, V);
}
double chi2inv(double x, double V)
{
	return gsl_cdf_chisq_Pinv(x, V);
}
