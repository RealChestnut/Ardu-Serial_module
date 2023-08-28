/***
 * This example expects the serial port has a loopback on it.
 *
 * Alternatively, you could use an Arduino:
 *
 * <pre>
 *  void setup() {
 *    Serial.begin(<insert your baudrate here>);
 *  }
 *
 *  void loop() {
 *    if (Serial.available()) {
 *      Serial.write(Serial.read());
 *    }
 *  }
 * </pre>
 */

#include <iostream>
#include <ros/ros.h>
#include <serial/serial.h>
#include <std_msgs/String.h>
#include <std_msgs/Empty.h>
#include <string.h>
#include <string>
#include <vector>
#include <std_msgs/Float32MultiArray.h>
#include <algorithm>
#include <iterator>


using namespace std;
serial::Serial ser;

static char sSTX() { return static_cast<char>(0x02);}
static char sETX() { return static_cast<char>(0x03);}
static char sACK() { return static_cast<char>(0x06);}
static char sNAK() { return static_cast<char>(0x15);}

bool back = false;
bool front = false;
bool hole_in_one = false;
const int numChars = 100;

int topic_num = 12;
string tempChars[numChars];
string receivedChars[numChars];
string messageFromMain[numChars];
string temp_result;
string buffer="";
string temp_1;
string temp_2;
string dot = ",";
string startMarker = "<";
string endMarker = ">";
string blink ="";

vector<string> last_dump;
vector<double> DoubleVec;

void receive_data();
void parseData(const string& str, vector<string>& values, string& delimiter);
void write_callback(const std_msgs::String::ConstPtr& msg){
    ROS_INFO_STREAM("Writing to serial port" << msg->data);
    ser.write(msg->data);
}



int main (int argc, char** argv){
    ros::init(argc, argv, "serial_example_node");
    ros::NodeHandle nh;

    ros::Subscriber write_sub = nh.subscribe("write", 1000, write_callback);
    ros::Publisher read_pub = nh.advertise<std_msgs::String>("read", 1);

    try
    {
        ser.setPort("/dev/ttyS0");
        ser.setBaudrate(115200);
        serial::Timeout to = serial::Timeout::simpleTimeout(1);
        ser.setTimeout(to);
        ser.open();
    }
    catch (serial::IOException& e)
    {
        ROS_ERROR_STREAM("Unable to open port ");
        return -1;
    }

    if(ser.isOpen()){
        ROS_INFO_STREAM("Serial Port initialized");
    }else{
        return -1;
    }

    ros::Rate loop_rate(500);
    while(ros::ok()){

        ros::spinOnce();
        
	if(ser.available()){
            ROS_INFO_STREAM("Reading from serial port");
            
 	    std_msgs::Float32MultiArray result;


	    receive_data();

            parseData(temp_result, last_dump ,dot);

	    //cout << last_dump.size() << endl; 
	
	    if(last_dump.size()==topic_num)
	    {

		   transform(last_dump.begin(), last_dump.end(), std::back_inserter(DoubleVec),
				   [&](string s){
				   return stod(s);
				   });
		for(int i =0; i<DoubleVec.size(); i++)
		{
		//	ROS_INFO_STREAM(" TEST : " << DoubleVec[i]);
		}	
		   /*	
		   result.layout.dim.push_back(std_msgs::MultiArrayDimension());
		   result.layout.dim[0].size = last_dump.size();
		   result.layout.dim[0].stride = 1;
		   result.data.clear();
		   result.data.insert(result.data.end(), floatVec.begin(), floatVec.end());
		   */
	    }
	    
	    last_dump.clear();
	    DoubleVec.clear();
		
	    
	}
	loop_rate.sleep();
    }
    	
}


void parseData(const string& str, vector<string>& values, string& delimiter){
	string msg;
	std_msgs::String buffer;
	if(str.empty()){return ;}
	if(str.front() != startMarker[0] ){return;}
	if(str.back() != endMarker[0]){return ;}
		
	
	msg.assign(str);
       	msg.erase(find(msg.begin(),msg.end(),'<'));
	msg.erase(find(msg.begin(),msg.end(),'>'));
	//ROS_INFO_STREAM("TEST : " <<msg);	
	//if(msg.size()>20 || msg.size()<20){return;}

	//ROS_INFO_STREAM("TESTING : " << msg);
	//ROS_INFO_STREAM("ERASE_TESTING : " << msg.begin() << " && " << msg.end());
	string::size_type Fpos = msg.find_first_not_of(delimiter,0);
	string::size_type Lpos = msg.find_first_of(delimiter, Fpos);
	
	while (string::npos != Fpos || string::npos != Lpos)
	{
		
	
		values.push_back(msg.substr(Fpos, Lpos - Fpos));
		
		

		Fpos = msg.find_first_not_of(delimiter, Lpos);
		Lpos = msg.find_first_of(delimiter, Fpos);
		
		//ROS_INFO_STREAM("VALUE[0] : " << values[0]);
		//ROS_INFO_STREAM("test :: " << Fpos << "&&" << Lpos);
	}
}

void receive_data(){
	
	    bool recvInProgress = false;

            buffer= ser.read(ser.available());
            size_t found_start = buffer.find("<");
            size_t found_end = buffer.find(">");
	    
	    //cout << found_start <<endl;
	    
	    
	    if(found_start != string::npos){
		    
		   string dumm = find(buffer.begin(),buffer.end(),'<');
		  cout << dumm << endl; 
		   // temp_1=buffer.substr(find(buffer.begin(),buffer.end(),'<'),buffer.end()+1);
	    }

 	    if((found_start == string::npos) && (found_end == string::npos)){

		    temp_1.append(buffer);

	    }

	    if(found_end != string::npos){


		    //temp_1.substr(buffer.begin(),buffer.find(">"));

		    temp_result = temp_1;

		    ROS_INFO_STREAM(" result test_1 : " << temp_1 << "\n");

		    temp_1.clear();

	    }



		
		    
	    /*	
	    if((found_start != string::npos) && (found_end != string::npos)){

                           temp_result=buffer;
                           hole_in_one = true;
                    }

            if((found_start != string::npos) && (hole_in_one == false)){


                    temp_1 = buffer;
                    front = true;

            }

            if( (found_end != string::npos) && (hole_in_one == false)){

                    temp_2 = buffer;
                    back = true;
            }

            if ((front==true) && (back == true)){
                    temp_result = temp_1 + temp_2;
                    front = false;
                    back =false;
            }

		*/


//            ROS_INFO_STREAM(" result buffer : " << buffer);
//            ROS_INFO_STREAM(" result test_1 : " << temp_1 << "\n");
//            ROS_INFO_STREAM(" result test_2 : " << temp_2);
            ROS_INFO_STREAM(" result test_result : " << temp_result << "\n");

	    temp_result.clear();
            hole_in_one = false;


}
