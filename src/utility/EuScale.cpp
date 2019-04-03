#include "EuScale.h"

namespace ul
{
void EuScale::getEuScaling(Range range, double &scale, double &offset)
{
	switch (range)
	{
	case BIP60VOLTS:
		scale = 120.0;
		offset = -(scale / 2.0);  //-60.0
		break;
	case BIP30VOLTS:
		scale = 60.0;
		offset = -(scale / 2.0);
		break;
	case BIP20VOLTS:
		scale = 40.0;
		offset = -(scale / 2.0);  //-20.0
		break;
	case BIP15VOLTS:
		scale = 30.0;
		offset = -(scale / 2.0);  //-30.0
		break;
	case BIP10VOLTS:
		scale = 20.0;
		offset = -(scale / 2.0);  //-10.0
		break;
	case BIP5VOLTS:
		scale = 10.0;
		offset = -(scale / 2.0);
		break;
	case BIP4VOLTS:
		scale = 8.0;
		offset = -(scale / 2.0);
		break;
	case BIP3VOLTS:
		scale = 6.0;
		offset = -(scale / 2.0);
		break;
	case BIP2PT5VOLTS:
		scale = 5.0;
		offset = -(scale / 2.0);
		break;
	case BIP2VOLTS:
		scale = 4.0;
		offset = -(scale/2.0);
		break;
	case BIP1PT25VOLTS:
		scale = 2.5;
		offset = -(scale / 2.0);
		break;
	case BIP1VOLTS:
		scale = 2.0;
		offset = -(scale / 2.0);
		break;
	case BIPPT625VOLTS:
		scale = 1.25;
		offset = -(scale / 2.0);
		break;
	case BIPPT5VOLTS:
		scale = 1.0;
		offset = -(scale / 2.0);
	  break;
	case BIPPT312VOLTS:
		scale = .625;
		offset = -(scale / 2.0);
		break;
	case BIPPT25VOLTS:
	  scale = 0.5;
	  offset = -(scale/2.0);
	  break;
	case BIPPT2VOLTS:
	  scale = 0.4;
	  offset = -(scale/2.0);
	  break;
	case BIPPT156VOLTS:
	  scale = 0.3125;
	  offset = -(scale / 2.0);
	  break;
	case BIPPT125VOLTS:
	  scale = 0.250;
	  offset = -(scale / 2.0);
	  break;
	case BIPPT1VOLTS:
		scale = 0.2;
		offset = -(scale / 2.0);
		break;
	case BIPPT05VOLTS:
		scale = 0.1;
		offset = -(scale / 2.0);
		break;
	case BIPPT01VOLTS:
		scale = 0.02;
		offset = -(scale / 2.0);
		break;
	case BIPPT005VOLTS:
		scale = 0.01;
		offset = -(scale / 2.0);
		break;
	case BIPPT078VOLTS:
	  scale = 0.15625;
	  offset = -(scale / 2.0);
	  break;


	case UNI60VOLTS:
		scale = 60.0;
		offset = 0.0;
		break;
	case UNI30VOLTS:
		scale = 30.0;
		offset = 0.0;
		break;
	case UNI20VOLTS:
		scale = 20.0;
		offset = 0.0;
		break;
	case UNI15VOLTS:
		scale = 15.0;
		offset = 0.0;
		break;
	case UNI10VOLTS:
		scale = 10.0;
		offset = 0.0;
		break;
	case UNI5VOLTS:
		scale = 5.0;
		offset = 0.0;
		break;
	case UNI4VOLTS:
	  scale = 4.0;
	  offset = 0.0;
	  break;
	case UNI2PT5VOLTS:
		scale = 2.5;
		offset = 0.0;
		break;
	case UNI2VOLTS:
		scale = 2.0;
		offset = 0.0;
		break;
	case UNI1PT25VOLTS:
		scale = 1.25;
		offset = 0.0;
		break;
	case UNI1VOLTS:
		scale = 1.0;
		offset = 0.0;
		break;
	case UNIPT625VOLTS:
		scale = 0.625;
		offset = 0.0;
		break;
	case UNIPT5VOLTS:
		scale = 0.5;
		offset = 0.0;
		break;
	case UNIPT25VOLTS:
	  scale = 0.25;
	  offset = 0.0;
	  break;
	case UNIPT2VOLTS:
		scale = 0.2;
		offset = 0.0;
		break;
	case UNIPT125VOLTS:
		scale = 0.125;
		offset = 0.0;
		break;
	case UNIPT1VOLTS:
		scale = 0.1;
		offset = 0.0;
		break;
	case UNIPT01VOLTS:
		scale = 0.01;
		offset = 0.0;
		break;
	case UNIPT078VOLTS:
		scale = 0.078;
		offset = 0.0;
		break;
	case UNIPT05VOLTS:
		scale = 0.05;
		offset = 0.0;
		break;
	case UNIPT005VOLTS:
		scale = 0.005;
		offset = 0.0;
		break;

	case MA0TO20:
		scale = 20.0;
		offset = 0.0;
		break;
	}
}
} /* namespace ul */
