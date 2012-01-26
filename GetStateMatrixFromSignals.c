#include <ansi_c.h>  
#include <cvirte.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>


void GetForcesFromLoadCellVoltages(double *lcVoltages,unsigned int numLoadCells,double *lcForces);
void GetShaftDisplacements(double *encoderAnglesPrevious,double *encoderAnglesCurrent,unsigned int numEncoders,
	double *shaftDisplacementsPrevious,double *shaftDisplacements);
void GetJR3FromVoltage(double *jr3Voltage, double *jr3Bias, double *jr3Output) ;
int GetStateMatrixFromSignals(int bufferInd, int numDataCols,double frameTime, double sampFreq,double *motorVoltages, double *encoderAnglesCurrent,
								double *lcVoltages,double *jr3Voltage,int numMarkers,char *viconString,double *encoderAnglesPrevious, double *jr3Bias,
								double *stateVector,double *stateMatrix, double *exportVars, int numExportVars) ;
void GetMarkerDataFromString(char *viconString,double *markerPositions);



int main (int argc, char *argv[]) 
{
	return 0;
}


void GetForcesFromLoadCellVoltages(double *lcVoltages,unsigned int numLoadCells,double *lcForces)
{
	double slope[20] = { 4669,    4757.48,    4995,    5001.4,    5137,    4510.86,    5084,    5062,    4527.04,    4616,    1,    10069,   1,    5471,   4953,1,5784.13,4984.46,1,1};
//	before calibration 12-9-11 = 	      { 4669,    4855,    4995,    4890,    5137,    4666,    5084,    5062,    4389,    4616,    1,    10069,   1,    5471,   4953,1,1,1,1,1};
//before calibration 11-14-11 { 4659,    4852,    5046,    5185,    5374,    4845,    5178,    5139,    4314,    4507,    1,    10069,   1,    4507,   5084,1,1,1,1,1};	
//	double slope[20] = 	      {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
//	double loadcellBias[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	
	
	double loadcellBias[20] = { -1.1585,  -10.613,   -4.2755,   -1.78,    4.5612,    5.45,  -10.9801,    0.2087,   -0.5747,    0.2136,    0.0,    1.6053,    0.0,   -0.2405,   43.5619,0,-1.965,-5.05,0,0};
//before calibration 11-14-11 { -0.7873,  -11.2109,   -4.3407,   -1.8723,    4.4399,    5.8396,  -11.1905,    0.4135,   -0.4715,    0.8625,    0.0,    1.6053,    0.0,   -0.7511,   44.5863,0,0,0,0,0};
	for(int lcNum = 0; lcNum < numLoadCells; lcNum++) 
	{
		lcForces[lcNum] = slope[lcNum]*lcVoltages[lcNum]+loadcellBias[lcNum];
		/* Force = slope*V + bias; */
	}
}


void GetShaftDisplacements(double *encoderAnglesPrevious,double *encoderAnglesCurrent,unsigned int numEncoders,
	double *shaftDisplacementsPrevious,double *shaftDisplacements)
{
	double shaftRadius = 3.175; // in mm
	double rad2DegFac = 57.2958; // conversion from radians to deg
	double *diff =   (double *) calloc(20,sizeof(double));
	for (int encNum = 0;encNum < numEncoders; ++encNum)
	{
		diff[encNum] = encoderAnglesCurrent[encNum] - encoderAnglesPrevious[encNum];
		if(diff[encNum] < -180.0)
		{
			diff[encNum] = diff[encNum] + 360.0; 
		}
		else 
		{
			if(diff[encNum] > 180.0)
			{
				diff[encNum] = diff[encNum] - 360.0; 
			}
		}
				
		 shaftDisplacements[encNum] = shaftDisplacementsPrevious[encNum] + shaftRadius*diff[encNum]/(rad2DegFac);
	}
	free(diff);
	
}

void GetJR3FromVoltage(double *jr3Voltage, double *jr3Bias, double *jr3Output)
{
	double jr3CalibMatrix[36] = {13.669,0.229,0.105,-0.272,0.06,0.865,
					  .16, 13.237,-0.387,-.027,-.396,-.477,
					  1.084,0.605,27.092,-.288,-.106,1.165,
					  -.007,-.003,-.001,.676,-.002,-.038,
					  0.004,-.004,0.001,0,0.688,-.012,
					  0.004,0.003,0.003,-.006,.013,.665};
	
/*	for(int i=0;i<6;++i)
	{
		jr3Output[i] = jr3Bias[i];
		for(int j=0;j<6;++j)
		{
			jr3Output[i] = jr3Output[i] + jr3CalibMatrix[6*i+j] * jr3Voltage[j] ;
		}
	}*/
	for(int i=0;i<6;++i)
	{
		jr3Output[i] = 0;
		for(int j=0;j<6;++j)
		{
			jr3Output[i] = jr3Output[i] + jr3CalibMatrix[6*i+j] * (jr3Voltage[j]- jr3Bias[j]);
		}
	}
	
	
}


void GetMarkerDataFromString(char *viconString,double *markerPositions)
{
	
	
	int lengthString;
	int commaCount =0;
	int startPos = 0;
	int endPos = 0;
	int charCount;
	int markerCount =0;
	int numChars;
	
	lengthString = strlen(viconString);
	
	int *positionsOfCommas = (int *) calloc(100,sizeof(int));
	
	for(int i=0;i<lengthString;i++)
	{
		
		if(viconString[i] == ',')
		{
			positionsOfCommas[commaCount] = i;
			commaCount++;	
			
		}
		
	}
	
 
	for(int j=1;j<commaCount-1;)
	{
		for(int k=0;k<3;++k)
		{
			startPos = positionsOfCommas[j+1+k]+1;
			endPos  =  positionsOfCommas[j+2+k]+1;
			numChars = endPos - startPos;
			if(numChars==1)
			{
				markerPositions[3*markerCount+k] = 1000000;
			}
			else
			{
					char tempString[30];				
				charCount=0;
				for(int p=startPos;p<endPos;++p)
				{
					tempString[charCount] = viconString[p];
					charCount++;
				}
			
				markerPositions[3*markerCount+k] = atof(tempString);	
			}
		}
		j=j+4;
		markerCount++;
		
	}
	free(positionsOfCommas);
}



int GetStateMatrixFromSignals(int bufferInd, int numDataCols,double frameTime, double sampFreq, double *motorVoltages, double *encoderAnglesCurrent,
								double *lcVoltages,double *jr3Voltage,int numMarkers,char *viconString,double *encoderAnglesPrevious, double *jr3Bias,
								double *stateVector,double *stateMatrix, double *exportVars, int numExportVars)
{												
	
		double shaftRadius = 3.175;
		double rad2DegFac = 57.2958;
		double maxDisplacement = 1000;   // mm 	 
		double maxForce = 25.0;		// newtons
		double maxVoltage = 8.0;	// Volts
	
		double initialGracePeriod = 3.0;
		int fail = -1;
		int numMotors = 20;
		unsigned int numLoadCells = 20;
		unsigned int numEncoders = 20;
		double lcForces[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		double shaftDisplacements[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		double shaftDisplacementsPrevious[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};		
		double jr3Output[6] = {0,0,0,0,0,0};				
		double *markerPositions = (double *) calloc(numMarkers,sizeof(double));
		
		for(int i=0;i<numMotors;++i)
		{ 
			shaftDisplacementsPrevious[i] = stateVector[i+1];
		}
		
		
		GetForcesFromLoadCellVoltages (lcVoltages,numLoadCells,lcForces);
		GetShaftDisplacements(encoderAnglesPrevious,encoderAnglesCurrent,numEncoders,shaftDisplacementsPrevious,shaftDisplacements) ;
		GetJR3FromVoltage(jr3Voltage, jr3Bias, jr3Output);
		GetMarkerDataFromString(viconString,markerPositions);
		
		
		
		//Filling the state matrix
		stateMatrix[bufferInd*numDataCols] = frameTime; 
		stateVector[0] = frameTime;
		for(int i=0;i<numMotors;++i)
		{
			stateVector[i+1]= shaftDisplacements[i];
			stateVector[numMotors + i+1]= lcForces[i];
			stateVector[2*numMotors + i+1]= motorVoltages[i];
			
			stateMatrix[bufferInd*numDataCols + i+1] = shaftDisplacements[i];
			stateMatrix[bufferInd*numDataCols +numMotors +  i+1]  =  lcForces[i];
			stateMatrix[bufferInd*numDataCols+2*numMotors + i+1]= motorVoltages[i];
		}
		
		for(int j=0;j<6;++j)
		{
			stateVector[3*numMotors +1+ j]=jr3Output[j];
			stateMatrix[bufferInd*numDataCols +3*numMotors + 1+j]  = jr3Output[j];
			
		} 
		
		for(int k=0;k<numMarkers*3;++k)
		{
			stateVector[3*numMotors+1+6+k] = markerPositions[k];
			stateMatrix[bufferInd*numDataCols +3*numMotors + 1+6+k] = markerPositions[k]; 
		}
		
		for(int l=0;l<numExportVars;++l) {
			stateVector[3*numMotors+1+6+3*numMarkers+l] = exportVars[l];
			stateMatrix[bufferInd*numDataCols +3*numMotors + 1+6+3*numMarkers+l] = exportVars[l];	
		}
		
		for (int i=0; i<20; ++i)
		{
			
//			rad2DegFac*sampFreq*(shaftDisplacements[i]-shaftDisplacementsPrevious[i])/shaftRadius > maxSpeed
			
			
			if ((fabs(shaftDisplacements[i]) > maxDisplacement) && (frameTime > initialGracePeriod)) 
			{
				fail = i;
				break;
			}
			
			if (!(i==17))
			{
				if ((fabs(lcForces[i]) > maxForce) && (frameTime > initialGracePeriod)) 
				{
					fail = i+20;
					break;
				}
			}
			if ((fabs(motorVoltages[i]) > maxVoltage) && (frameTime > initialGracePeriod))
			{
				fail = i+40;
				break;
			}
		}
		
		free(markerPositions);
		return fail;
			   
		
}


