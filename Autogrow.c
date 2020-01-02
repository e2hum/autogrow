#include "PC_FileIO.c"
const int NUM_CHECK = 4;
//period of time that passes between checking (IN SECOND).
//Can be set to a smaller value for demo purpose.
const int UNIT_TIME = 90;//3600 * 24 / NUM_CHECK;
const int MAX_PLANTS = 6;
const int MAX_DAYS = 16;
const int ROTATION = 360;
const int INCREMENT = 18;

//creates a schedule for when plant should be watered and water volume
//This 2D array Stores water_time data for each plants
int PlantInfo[MAX_DAYS*NUM_CHECK][MAX_PLANTS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Initializes neccesary arrays, reads files, set sensors
bool initialize(int & runTime, int & numPlants, string * plantName)
{
	//Set sensors
	SensorType[S1] = sensorEV3_Ultrasonic;
	//DECLARE AND SET ARRAYS
	int waterVolume[MAX_PLANTS] = {0,0,0,0,0,0};
	int waterFrequency[MAX_PLANTS] = {1,1,1,1,1,1};
	bool success=false;
	//READ FILES
	TFileHandle fin;
	bool fileOkay = openReadPC(fin, "PlantInfo.txt");
	if (!fileOkay) //checks if file opened
	{
		displayString(3,"File Not Found.");
		wait1Msec(10000);
	}
	else
	{
		readIntPC(fin, runTime);
		readIntPC(fin, numPlants);
		if(runTime > MAX_DAYS || runTime <= 0 || numPlants <= 0 || numPlants > MAX_PLANTS)
		{
			displayString(0,"Invalid Input!");
			wait1Msec(5000);
		}
		else
		{
			success=true;
			for (int count = 0; count < numPlants; count++)
			{
				/*string temp = "";
				readTextPC(fin, temp);
				(string)plantName[count] = temp;*/
				readIntPC(fin, waterVolume[count]);
				readIntPC(fin, waterFrequency[count]);
			}
		}
	}
	wait1Msec(1000);
	closeFilePC(fin);
	//Poplulate PlantInfo[][]
	if(success)
	{
		for(int index=0;index<numPlants;index++)
		{

			for(int count=0;count<runTime*NUM_CHECK;count++)
			{
				//
				if(count%waterFrequency[index]==0)
					PlantInfo[count][index]=waterVolume[index];
			}
		}
	}
	return success;
}

//Dispenses specific volumes of water
void dispenseWater(int waterDispensed, int waterUsed)
{
	waterDispensed -= 10; // Accounts for time of arm moving up and down
	int power = -40, reversePower = 40, fwdTime = 800, revTime = 800;
	//calculates time needed to turn on valve based off flow rate
	//mild calculus used here
	float rate = 0.5908;
	float timer = (rate*(waterUsed/100.0) + 5.254)*(waterDispensed/100.0) * 1000;

	//turns on the valve
	motor[motorC] = power;
	wait1Msec(fwdTime);
	motor[motorC] = 0;
	time1[T4] = 0;
	displayString(0, "timer: %f", timer);
	while(time1[T4] < timer)
	{}
	//turns off the valve
	motor[motorC] = reversePower;
	wait1Msec(revTime);
	motor[motorC] = 0;
}

//Alarm for when intruder is detected
void alarm()

{
	bool leave = false;
	time1[T1] = 0;
	while(SensorValue[S1] != 255 && time1[T1] < 10000 && leave == false) //alarm will sound for maximum 10 seconds
	{
		if (SensorValue[S1] > 30) //quiet, sporadic alarm if intruders are detected within 255 cm but greater than 30 cm
		{
			setSoundVolume(5);
			while (SensorValue[S1] > 30 && leave == false)
			{
				playSound(soundBeepBeep);
				wait1Msec(500);
				if (getButtonPress(buttonEnter) || Time1[T1] == 10000) //Press the enter button to disarm the alarm
				{
					clearSounds();
					leave = true;
				}
			}
		}
		else //loud, constant alarm if intruders are detected within 30 cm
		{
			setSoundVolume(100);
			while (!getButtonPress(ENTER_BUTTON))
			{
				playSound(soundBeepBeep);
			}
			while (getButtonPress(ENTER_BUTTON) || Time1[T1] == 10000) //Press the enter button to disarm the alarm
			{}
			clearSounds();
			leave = true;
		}
	}
}

//Rotate ultrasonic sensor +360 deg and then back to its starting
//position for evey x second. When intruder detected, call alarm function,
void detection(bool first, int * angleHolder, int dayNum,int *numIntruders)

{
	nMotorEncoder[motorB] = 0;
	int turnDeg = 0;
	// if this is the first time the detection function is being run
	// sets up an array of values for where objects are
	if (first)
	{
		motor[motorB] = 5;
		while (nMotorEncoder[motorB] < ROTATION)
		{
			if (nMotorEncoder[motorB] >= turnDeg*20)
			{
				angleHolder[turnDeg] = SensorValue[S1];
				turnDeg++;
			}
		}
		motor[motorB] = 0;
	}
	// compares values in the array to values scanned when at those positions
	// if the value it sees is less than value in the array it will activate the alarm function
	else
	{
		motor[motorB] = 5;
		const int ANGLE_TOL = 3;

		while (nMotorEncoder[motorB] < ROTATION)
		{

			if (nMotorEncoder[motorB] >= turnDeg*20)
			{
				if (abs(angleHolder[turnDeg] - SensorValue[S1]) > ANGLE_TOL)
				{
					int temp = SensorValue[S1];
					//displayString(12, "angle held %d", angleHolder[turnDeg]);
					//displayString(13, "sensor value %d", temp);
					motor[motorB] = 0;
					alarm();
					numIntruders[dayNum]++;
					motor[motorB] = 5;
				}
				turnDeg++;
			}
		}
		motor[motorB] = 0;
	}
	motor[motorB] = -5;
	while (nMotorEncoder[motorB] > 0)
	{}
	motor[motorB] = 0;

}

//moves the arm to the next spot
//calculates distance and angle based of the number of plants
//accounts for one spot where the robot arm rests
void moveOne(int numPlants)
{
	//angle to get to next spot
	const int ENC_CONVERT = 180/(2.75*PI);
	int circumference = 2*PI*48.2 + 10;
	float increment = circumference/(numPlants+1);

	nMotorEncoder[motorA] = 0;
	wait1Msec(100);
	motor[motorA] = 30;
	while(nMotorEncoder[motorA] < increment*ENC_CONVERT)
	{}
	motor[motorA] = 0;
	wait1Msec(1000);
}

//Function Move Arm
//This function moves robot arm from current to target position
//movement is only in direction and always goes to null position after
//full rotation thus only one case checked
void moveArm(int position,int plantNumber,int numPlants){
	int delta =0;
	//if the arm moves from null position to plant position
	if (position == numPlants)
	{
		//special delta calculation
		delta = plantNumber +1;
	}
	else {
		delta = plantNumber-position;
	}
	//moves the number of times required
	for (int x = 0; x<delta;x++)
		moveOne(numPlants);
}

//RECIEVE CHECKPOINT, the row in the plantInfo(schedule) array to check
//checks if watering is needed, moves to location, dispenses water, keeps track of position
//move robot arm to staring position
void water(int numPlants, int * totalWaterRecieved, int checkPoint,int &waterUsed)
{
	int position=numPlants;//sets initial arm position
	//note that numPlants is the also the position for the empty spot
	for (int plantNumber=0;plantNumber<numPlants;plantNumber++)
	{
		if(PlantInfo[checkPoint][plantNumber] !=0 )
		{
			//moves, waters, and updates values
			moveArm(position, plantNumber, numPlants);
			dispenseWater(PlantInfo[checkPoint][plantNumber],waterUsed);
			waterUsed+=PlantInfo[checkPoint][plantNumber];
			totalWaterRecieved[plantNumber]+=PlantInfo[checkPoint][plantNumber];
			position = plantNumber;
		}
	}
	//moves arm to starting positions
	moveArm(position, numPlants, numPlants);
}

//displays volume of water that the plants have recieve
void displayCurrent(string * plantName, int * totalWaterRecieved, int numPlants, int * numIntruders )
{
	eraseDisplay();
	displayString(2,"GROUP_803 IS AWESOME!");
	displayString(3,"Plant Name     Total Water");
	for(int index=0;index<numPlants;index++)
	{
		string plant_name = (string)plantName[index];
		int total_water = totalWaterRecieved[index];
		displayString(5+index," %s:        %d ml", plant_name, total_water);
	}
	int total = 0;
	for (int x = 0; x<MAX_DAYS;x++)
	{
		total+= numIntruders[x];
		}
	displayString(13,"Number of Intruders: %d",total);
}

//performs one full rotation and beeps to indicate where plants should be located
void setPositions(int numPlants){
	for (int x = 0;x<numPlants;x++){
		moveOne(numPlants);
		playSound(soundBlip);
		wait1Msec(250);
		playSound(soundBlip);
		wait1Msec(3000);
	}
	moveOne(numPlants);
}


task main()
{
	int runTime=0; //# of days
	int numPlants=0; //# of plants
	string plantName[MAX_PLANTS]={"A","B","C","D","E","F"};
	int totalWaterRecieved[MAX_PLANTS]={0,0,0,0,0,0}; //Cumilative amount of water for each plants
	int angleHolder[INCREMENT] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int numIntruders[MAX_DAYS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //number of intruderes detected each day
	int waterUsed=0;

	displayCurrent(plantName,totalWaterRecieved,4, numIntruders);


	if (initialize(runTime,numPlants,plantName))
	{
		displayString(4,"Press any Button to start");
		while(!getButtonPress(buttonAny))
		{}
		while(getButtonPress(buttonAny))
		{}
		eraseDisplay();
		int rowCheck = 0;

		//setup functions
		detection(1,angleHolder, 0, numIntruders);
		wait1Msec(3000);

		setPositions(numPlants);
		wait1Msec(1500);
		for (int x = 0; x < 5 ; x++)
		{
			setSoundVolume(10);
			playSound(soundBlip);
			wait1Msec(300);
		}


//==================OPERATION START=====================
		time100[T2]=0;//timer ticks every 0.1 s
		// This timeTotal should be used for proper operation
		//int timeTotal=runTime*NUM_CHECK*UNIT_TIME*10;//total runtime in 0.1s increments

		// This timeTotal is for demo purposes (set to 1 minute)
		int timeTotal = 700;
		displayCurrent( plantName,totalWaterRecieved,numPlants,numIntruders);


		while(time100[T2]<timeTotal)//operation time
		{
			// Since program may be in while loop for long periods of time, rechecks the intial while condition
			// so that program ends at specified time
			// while loop exits when either total time is reached or time to check if plants need water is reached

		while(time100[T2] < timeTotal && time100[T2] < UNIT_TIME*10*rowCheck)
			{
				displayString(10,"timer %d total time %d", time100[T2], timeTotal);
				//while not checking for water, runs detection
				detection(0, angleHolder,rowCheck/NUM_CHECK +1, numIntruders);
				wait1Msec(400); // buffer between detections
			}

			// only waters if time limit has not been reached
			if(time100[T2]<timeTotal)
				water(numPlants,totalWaterRecieved,rowCheck, waterUsed);

			displayCurrent(plantName,totalWaterRecieved,numPlants,numIntruders);
			rowCheck++;
		}
	}

	//===============END OF OPERATIONS==================



		for (int x = 0; x < 10 ; x++)
		{
			setSoundVolume(50);
			playSound(soundBlip);
			wait1Msec(300);
		}
	while(!getButtonPress(ANY_BUTTON))
	{}
	while(getButtonPress(ANY_BUTTON))
	{}
}
