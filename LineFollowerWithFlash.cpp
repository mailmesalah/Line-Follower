#include "MSCFileSystem.h"
#include "m3pi.h"
#include "mbed.h"

m3pi m3pi;
MSCFileSystem msc("msc");

DigitalOut relay (p20);
DigitalIn button21(p21);
DigitalIn button25(p25);

// Minimum and maximum motor speeds
#define MAX 0.3
#define MIN 0

//variable for storing speeds of the motors
float highSpeed = MAX;
float lowSpeed=0.0;


void moveMotor(float currentPos)
{

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

void charge (float volts)
{

    volatile float currentVolts = m3pi.battery();

    if (currentVolts < volts) { // needs charging
        relay = 1; // switch relay on
        while (currentVolts < volts) {
            currentVolts = m3pi.battery();
            m3pi.cls();
            m3pi.locate(0,0);
            m3pi.printf("%.2f",currentVolts);
            wait(10.0);
        }
        relay = 0; // switch relay off
        return;

    } else { // no charege needed
        return;
    }

}


int main()
{

    button21.mode(PullUp);
    button25.mode(PullUp);
    //wait for 2 seconds before calibrating
    volatile float currentVolts = m3pi.battery();
    m3pi.cls();
    m3pi.locate(0,0);
    m3pi.printf("%.2f",currentVolts);

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
    int delayCounter=0;

    FILE *fw = fopen("/msc/record.dat", "w");
    FILE *fr;
    while (1) {


        // Get the position of the line.
        if(!readCompleted) {
            //get position from the sensors
            currentPos = m3pi.line_position();
            fprintf(fw, "%f\n",currentPos);

            //reset timer
            //t.stop();
            //t.start();
        } else {

            if(delayCounter>=5) {
                delayCounter=0;

                if(fscanf(fr,"%f",&currentPos)==EOF) {
                    //stops the motors
                    m3pi.stop();

                    fclose(fr);
                    //get out of the loop
                    break;
                }
                if(currentPos>=10.0) {
                    //if end of one recording
                    m3pi.stop();
                    //wait for 2 seconds                    
                    wait(2.0);
                    fscanf(fr,"%f",&currentPos);
                }
            }
            ++delayCounter;


        }

        moveMotor(currentPos);

        //increment count for delay calculating
        count=count+1;

        if(count==500) {
            //resets the count sp that it counts again from 0
            count=0;

            //time counting
            ++timeCounter;
            //check the time to stop
            if(timeCounter>=2 && !readCompleted) {
                timeCounter=0;
                //Stops the motors at the end (all black for all sensors is considered end)
                m3pi.stop();
                //Sets the read is over so that it can prepare to draw the path it saved


                //wait for button to be clicked
                while(1) {
                    if (!button21) {
                        //Record                        
                        fprintf(fw, "%f\n",10.0);
                        break;
                    }

                    if(!button25) {
                        //Draw
                        readCompleted=1;
                        fclose(fw);
                        //wait for the button to click
                        fr = fopen("/msc/record.dat", "r");

                        break;
                    }
                }
                //wait for 2 secs before drawing
                wait(2.0);

            }

        }

    }

}
