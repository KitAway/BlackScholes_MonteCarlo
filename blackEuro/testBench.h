/*----------------------------------------------------------------------------
*
* Author:   Liang Ma (liang-ma@polito.it)
*
* Several input parameters for the simulation are defined in thie file.
*
* S0:		stock price at time 0
* K:		strike price
* rate:		interest rate
* volatility:		volatility of stock
* T:		time period of the option
* kernel_name:	the kernel name
*
*----------------------------------------------------------------------------
*/

#include "../common/stockData.h"

namespace TB 
{
	const double S0 = 100;
	const double K = 110;
	const double rate = 0.05;
	const double volatility = 0.2;
	const double T = 1.0;
	stockData data(T,rate,volatility,S0,K);
}
