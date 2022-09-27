#include "mbed.h"
#include "m3pi.h"
#include <vector>


m3pi m3pi;
Timer t;
// Minimum and maximum motor speeds
#define MAX 0.3
#define MIN 0

//variable for storing speeds of the motors
float highSpeed = MAX;
float lowSpeed=0.0;

void moveMotor(float currentPos){
    
    //Check line position and calculating speed of left and right motors
        if(currentPos<0) {            
            //line is to the left of the sensors

            //Calculating the speed
            lowSpeed=(currentPos+MAX)*-1;
            

            //assigning the speeds to motors
            m3pi.left_motor(highSpeed);
            m3pi.right_motor(lowSpeed);
                        
        } else if(currentPos>=0) {
            //line is to right of the sensors or to the center

            //calculating the speed
            lowSpeed=MAX-currentPos;
            
            
            //assigning the speeds to motors
            m3pi.left_motor(lowSpeed);
            m3pi.right_motor(highSpeed);
                        
        }
    
}

int main()
{
    

   //wait for 2 seconds before calibrating
    wait(2.0);

    //Auto calibrates the position of line with respect to the photo sensors
    m3pi.sensor_auto_calibrate();


    //current position of the line under sensor
    float currentPos = 0.0;
    

    //For counting iteration for printing values with proper delay
    int count=0;
    int timeCounter=0;

    //used to check if read path has been over or not
    int readCompleted=0;

    std::vector<float> storePos;
    int storeIndex=0;   
    int delayCounter=0;
    
    //int deviation=0;
    
    
    t.start();
    while (1) {

        // Get the position of the line.
        if(!readCompleted){            
            //get position from the sensors
            currentPos = m3pi.line_position();            
            storePos.push_back(currentPos);
            //storePos.push_back(t.read());
            //reset timer
            t.stop();
            t.start();
        }
        else{
            //get postion from memory
            if(storePos.size()>storeIndex){
                if(delayCounter>=5)
                {
                    currentPos=storePos.at(storeIndex);
                    ++storeIndex;                
                    delayCounter=0;
                }
                ++delayCounter;
            }
            else{
                //stops the motors
                m3pi.stop();  
                //get out of the loop
                break;    
            }
            
        }
        
        moveMotor(currentPos);
        //delaying
        /*if(readCompleted){
            wait(0.3);
            //++storeIndex;
        }*/
        //increment count for delay calculating
        count=count+1;

        if(count==500) {
            //resets the count sp that it counts again from 0
            count=0;

            //Sets the cursor of LCD to second row and second column
            m3pi.locate(1,1);
            //Prints the current possition floating point number to the LCD
            m3pi.printf("%f",currentPos);
            
            //time counting
            ++timeCounter;
            //check the time to stop
            if(timeCounter>=3){
                //Stops the motors at the end (all black for all sensors is considered end)
                m3pi.stop();
                //Sets the read is over so that it can prepare to draw the path it saved
                readCompleted=1;
          
                //wait for the button to click
          
                //wait for 2 secs before drawing
                wait(2.0);
                timeCounter=-1000;
            }

        }


    }
}