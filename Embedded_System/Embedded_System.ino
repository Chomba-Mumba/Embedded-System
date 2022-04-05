#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
//structures for menus
typedef enum cursor_pos_main{non,ground,first,outside,details,kitchen,hall,living,bed1,bed2,bath,garage,garden} pos;
typedef enum cursor_pos_device{null,lamp_bed1,lamp_bed2,lamp_bath,lamp_kitchen,lamp_hall,lamp_liv,light_bed1,heat_bed1,light_bed2,heat_bed2,heat_bath,light_bath,heat_kitchen,light_kitchen,heat_liv,light_liv,heat_hall,light_hall,water_garden,light_garage} pos_device;
typedef enum room_loc{nun,bed_room1,bed_room2,Outside,bath_room,kitchen_room,hall_room,liv_room} rooms;
typedef enum cursor_pos_details{nan,onTime,offTime,level} pos_details;

//data structure stored in array
typedef struct timings {int hours; int minutes;} timing;
typedef struct value {timing On_Time; timing Off_Time; int Level;} values;
typedef struct data1 {char room[10]; char Name[6]; values val;} room_data1;
typedef struct data {char room[10]; char type[6]; values val;} room_data;

//global variable for cursor position on men
pos_details cursor1_det = nan;
pos_device cursor1_dev = null;
pos cursor1 = ground;
rooms room = nun;
bool first_pass = true;
bool value_Screen = false;
bool timePos = true;
//position of array value that needs changing
int valueToChange;
int valueToChangeLevel;
bool On_Time_Updated = false;
bool Off_Time_Updated = false;

//function to move the cursor around the menu, true places cursor in top position
void move_cursor(String x,String y,bool z){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(x);
  if (z){
    lcd.print(F("<"));
    lcd.setCursor(0,1);
    lcd.print(y);
  } else {
    lcd.setCursor(0,1);
    lcd.print(y);
    lcd.print(F("<"));
  }
}
//function to add padding (with 0s) for time if needed

String changeToTime(int x){
  String z = "0";
  char t[10];
  String y;
  if (x>9){
    sprintf(t,"%d",x);
    y = t;
  }else {
    sprintf(t,"%d",x);
    y = z + t;  
  }
  return y;
}
//function to print out the floor of the device
void printFloor(room_data item){
  if ((strcmp(item.room,"Bedroom")==0)||(strcmp(item.room,"Bedroom 2")==0)||(strcmp(item.room,"Bathroom")==0)){
    Serial.print(F("Ground"));
  } else if((strcmp(item.room,"Kitchen")==0)||(strcmp(item.room,"Hall")==0)||(strcmp(item.room,"Living")==0)){
    Serial.print(F("First"));
  } else if ((strcmp(item.room,"Garage")==0)||(strcmp(item.room,"Garden")==0)){
    Serial.print(F("Outside"));
  }
} 
//function to prnt out all the values of the array

//function to print out the whole data structure
void printall(room_data room_array2[],int arraySize,room_data1 lamp_array[],int arraySize1){
  int i = 0;
  int j = 0;
  for (i;i<14;i++){
    printFloor(room_array2[i]);     
    Serial.print(F("/"));
    Serial.print(room_array2[i].room );
    Serial.print(F("/"));
    Serial.print(room_array2[i].type);
    Serial.print(F("/Main/On:"));
    Serial.println(changeToTime(room_array2[i].val.On_Time.hours) + "." + changeToTime(room_array2[i].val.On_Time.minutes));

    printFloor(room_array2[i]);
    Serial.print(F("/"));
    Serial.print(room_array2[i].room );
    Serial.print(F("/"));
    Serial.print(room_array2[i].type);
    Serial.print(F("/Main/Off:"));
    Serial.println(changeToTime(room_array2[i].val.Off_Time.hours) + "." + changeToTime(room_array2[i].val.Off_Time.minutes));

    if (!(strcmp(room_array2[i].type,"Water")==0)){
      printFloor(room_array2[i]);        
      Serial.print(F("/"));
      Serial.print(room_array2[i].room );
      Serial.print(F("/"));
      Serial.print(room_array2[i].type);
      Serial.print(F("/Main/Level:"));
      Serial.println(room_array2[i].val.Level);
      //print out lamp values after heat values
      if (strcmp(room_array2[i].type,"Heat")==0){
        printFloor(room_array2[i]);       
        Serial.print(F("/"));
        Serial.print(lamp_array[j].room );
        Serial.print(F("/Lamp/"));
        Serial.print(lamp_array[j].Name);
        Serial.print(F("/On:"));
        Serial.println(changeToTime(lamp_array[j].val.On_Time.hours) + "." + changeToTime(lamp_array[j].val.On_Time.minutes));

        printFloor(room_array2[i]);    
        Serial.print(F("/"));
        Serial.print(lamp_array[j].room );
        Serial.print(F("/Lamp/"));
        Serial.print(lamp_array[j].Name);
        Serial.print(F("/Off:"));
        Serial.println(changeToTime(lamp_array[j].val.Off_Time.hours) + "." + changeToTime(lamp_array[j].val.Off_Time.minutes));
      
        printFloor(room_array2[i]);           
        Serial.print(F("/"));
        Serial.print(lamp_array[j].room );
        Serial.print(F("/Lamp/"));
        Serial.print(lamp_array[j].Name);
        Serial.print(F("/Level:"));
        Serial.println(lamp_array[j].val.Level);
        j +=1;
      }
    }
  }
}
//function to adjust values depending on the button pressed
int changeValue(int x,String format){
  int button_state = lcd.readButtons();
  int upperLimit;
  int lowerLimit;
  if (format=="hours"){
    upperLimit = 23;
    lowerLimit = 0;
  }else if (format == "minutes"){
    upperLimit = 59;
    lowerLimit = 0;
  }else if (format == "level"){
    upperLimit = 100;
    lowerLimit = 0;
  }
    if (button_state & BUTTON_UP){
      if (x == upperLimit){
        x = lowerLimit;
      }else {
        x +=1;
      }
    }else if (button_state & BUTTON_DOWN){
      if (x == lowerLimit){
        x = upperLimit;
      }else {
        x -=1;
      }
    }
  return x;       
}


//change the cursor position on the main menu depending on previous button movement
void slct(String x, String y,String z,pos position1,pos position2,pos position3){
  if (first_pass){
    move_cursor(x,y,true);
    first_pass = false;
  }
  int button_state = lcd.readButtons();
  if (cursor1==position1){
    if (button_state & BUTTON_DOWN){
      move_cursor(x,y,false);
      cursor1=position2;
    }else if (button_state & BUTTON_UP){
      move_cursor(y,z,false);
      cursor1 = position3;
    }
  } else if (cursor1 == position2) { 
      if (button_state & BUTTON_UP){
        move_cursor(x,y,true);
        cursor1=position1;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(y,z,false);
        cursor1=position3;
      }
  }else if (cursor1 == position3){
      if (button_state & BUTTON_UP){
        move_cursor(y,z,true);
        cursor1=position2;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(x,y,true);
        cursor1=position1;
      }
  } 
}

void slct1(String a, String b, String c, String d,pos position1,pos position2,pos position3,pos position4){
  if (first_pass){
    move_cursor(a,b,true);
    first_pass = false;
  } 
  int button_state = lcd.readButtons();
  if (cursor1==position1){
    if (button_state & BUTTON_DOWN){
      move_cursor(a,b,false);
      cursor1=position2;
    }else if (button_state & BUTTON_UP){
      move_cursor(c,d,false);
      cursor1 = position4;
    }
  }else if (cursor1 == position2) { 
      if (button_state & BUTTON_UP){
        move_cursor(a,b,true);
        cursor1=position1;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(b,c,false);
        cursor1=position3;
      }
  }else if (cursor1 == position3){
      if (button_state & BUTTON_UP){
        move_cursor(b,c,true);
        cursor1=position2;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(c,d,false);
        cursor1=position4;
      }
  }else if (cursor1 == position4){
      if (button_state & BUTTON_UP){
        move_cursor(c,d,true);
        cursor1=position3;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(a,b,true);
        cursor1=position1;
      }
  }
}

void slct2(String x, String y, String z,pos_device position1,pos_device position2, pos_device position3){
  if (first_pass){
    move_cursor(x,y,true);
    first_pass = false;
  }
  int button_state = lcd.readButtons();
  if (cursor1_dev==position1){
    if (button_state & BUTTON_DOWN){
      move_cursor(x,y,false);
      cursor1_dev=position2;
    }else if (button_state & BUTTON_UP){
      move_cursor(y,z,false);
      cursor1_dev = position3;
    }
  } else if (cursor1_dev == position2) { 
      if (button_state & BUTTON_UP){
        move_cursor(x,y,true);
        cursor1_dev=position1;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(y,z,false);
        cursor1_dev=position3;
      }
  }else if (cursor1_dev == position3){
      if (button_state & BUTTON_UP){
        move_cursor(y,z,true);
        cursor1_dev=position2;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(x,y,true);
        cursor1_dev=position1;
      }
  } 
}

void slct3(String x, String y,String z,pos_details position1,pos_details position2,pos_details position3){
  if (first_pass){
    move_cursor(x,y,true);
    first_pass = false;
  }
  int button_state = lcd.readButtons();
  if (cursor1_det==position1){
    if (button_state & BUTTON_DOWN){
      move_cursor(x,y,false);
      cursor1_det=position2;
    }else if (button_state & BUTTON_UP){
      move_cursor(y,z,false);
      cursor1_det = position3;
    }
  } else if (cursor1_det == position2) { 
      if (button_state & BUTTON_UP){
        move_cursor(x,y,true);
        cursor1_det=position1;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(y,z,false);
        cursor1_det=position3;
      }
  }else if (cursor1_det == position3){
      if (button_state & BUTTON_UP){
        move_cursor(y,z,true);
        cursor1_det=position2;
      }else if (button_state & BUTTON_DOWN){
        move_cursor(x,y,true);
        cursor1_det=position1;
      }
  } 
}

void slct4(String x, String y,pos_details position1,pos_details position2){
  if (first_pass){
    move_cursor(x,y,true);
    first_pass = false;
  }
  int button_state = lcd.readButtons();
  if (cursor1_det==position1){
    if (button_state & BUTTON_DOWN|| button_state & BUTTON_UP){
      move_cursor(x,y,false);
      cursor1_det=position2;
    }
  }else if (cursor1_det == position2) { 
    if (button_state & BUTTON_UP|| button_state & BUTTON_DOWN){
      move_cursor(x,y,true);
      cursor1_det=position1;
    }
  }
}

void slct5(String x, String y,pos_device position1,pos_device position2){
  if (first_pass){
    move_cursor(x,y,true);
    first_pass = false;
  }
  int button_state = lcd.readButtons();
  if (cursor1_dev==position1){
    if (button_state & BUTTON_DOWN|| button_state & BUTTON_UP){
      move_cursor(x,y,false);
      cursor1_dev=position2;
    }
  }else if (cursor1_dev == position2) { 
    if (button_state & BUTTON_UP|| button_state & BUTTON_DOWN){
      move_cursor(x,y,true);
      cursor1_dev=position1;
    }
  }
}

//can remove last parameter from this
void changeScreen(pos_device cursor_pos1,pos_device cursor_pos2,pos_device cursor_pos3,rooms roomChange,String pos1, String pos2, String pos3, pos cursor1_pos){
  cursor1 = non;
  first_pass = true;
  cursor1_dev = cursor_pos1;
  room = roomChange;
  slct2(pos1,pos2,pos3,cursor_pos1,cursor_pos2,cursor_pos3);
}

void changeScreen1(pos_device cursor_pos1,pos_device cursor_pos2,rooms roomChange,String pos1, String pos2, pos cursor1_pos){
  cursor1 = non;
  first_pass = true;
  cursor1_dev = cursor_pos1;
  room = roomChange;
  slct5(pos1,pos2,cursor_pos1,cursor_pos2);
}
//screen for changing level
int levelScreen(pos_device state,room_data room_array[],int arraySize,int button_state){
  int outputLevel;
  int j = 0;
  for (j;j<14;j++){
    //loop through the array to find the correct struct with the correct data
    switch (state){
      case light_garage:
        if (strcmp("Garage",room_array[j].room)==0 & strcmp("Light", room_array[j].type)==0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level; 
        }
        break;
      case light_bed1:
        if (strcmp("Bedroom",room_array[j].room)==0 & strcmp("Light", room_array[j].type)==0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;      
        }
        break;
      case heat_bed1:
        if (strcmp(room_array[j].room,"Bedroom")==0 & strcmp(room_array[j].type,"Heat")==0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case light_bed2:
        if (strcmp(room_array[j].room,"Bedroom 2") == 0 & strcmp(room_array[j].type,"Light") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case heat_bed2:
        if (strcmp(room_array[j].room,"Bedroom 2") == 0 & strcmp(room_array[j].type,"Heat") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case heat_bath:
        if (strcmp(room_array[j].room,"Bathroom") == 0 & strcmp(room_array[j].type,"Heat") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case light_bath:
        if (strcmp(room_array[j].room,"Bathroom") == 0 & strcmp(room_array[j].type,"Light") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case heat_kitchen:
        if (strcmp(room_array[j].room,"Kitchen") == 0 & strcmp(room_array[j].type,"Heat") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");          
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case light_kitchen:
        if (strcmp(room_array[j].room,"Kitchen") == 0 & strcmp(room_array[j].type,"Light") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case heat_liv:
        if (strcmp(room_array[j].room,"Living") == 0 & strcmp(room_array[j].type,"Heat") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case light_liv:
        if (strcmp(room_array[j].room,"Living") == 0 & strcmp(room_array[j].type,"Light") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case heat_hall:
        if (strcmp(room_array[j].room,"Hall") == 0 & strcmp(room_array[j].type,"Heat") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case light_hall:
        if (strcmp(room_array[j].room,"Hall") == 0 & strcmp(room_array[j].type,"Light") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      }
    }
    if (first_pass){
      first_pass = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("Level: "));
      lcd.print(outputLevel);
    }
  
    if ((button_state & BUTTON_UP)||(button_state & BUTTON_DOWN)){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("Level: "));
      lcd.print(outputLevel);
    }
  return outputLevel;
}

int levelScreenLamp(pos_device state,room_data1 room_array[],int arraySize,int button_state){
  int outputLevel;
  int j = 0;
  for (j;j<6;j++){
    //loop through the array to find the correct struct with the correct data
    switch (state){
      case lamp_bed1:
        if (strcmp("Bedroom",room_array[j].room)==0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;      
        }
        break;
      case lamp_bed2:
        if (strcmp(room_array[j].room,"Bedroom 2") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case lamp_bath:
        if (strcmp(room_array[j].room,"Bathroom") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case lamp_kitchen:
        if (strcmp(room_array[j].room,"Kitchen") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case lamp_liv:
        if (strcmp(room_array[j].room,"Living") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      case lamp_hall:
        if (strcmp(room_array[j].room,"Hall") == 0){
          room_array[j].val.Level = changeValue(room_array[j].val.Level,"level");
          valueToChangeLevel = j;
          outputLevel = room_array[j].val.Level;
        }
        break;
      }
    }
    if (first_pass){
      first_pass = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("Level: "));
      lcd.print(outputLevel);
    }
  
    if ((button_state & BUTTON_UP)||(button_state & BUTTON_DOWN)){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("Level: "));
      lcd.print(outputLevel);
    }
  return outputLevel;
}

// screen for changing time
timing timeScreen(pos_device state, pos_details pos_state,room_data room_array[],int arraySize,int button_state){
  String hours;
  String minutes;
  timing output;
  int m = 0;
  for (m;m<14;m++){
    if (pos_state==onTime){
      On_Time_Updated = true;
      Off_Time_Updated = false;
      //loop through the array to find the correct struct with the correct data
      switch (state){
        case water_garden:
          if (strcmp(room_array[m].room,"Garden")==0 & strcmp(room_array[m].type, "Water")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case light_garage:
          if (strcmp(room_array[m].room,"Garage")==0 & strcmp(room_array[m].type, "Light")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case light_bed1:
          if (strcmp(room_array[m].room,"Bedroom")==0 & strcmp(room_array[m].type, "Light")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case heat_bed1:
          if (strcmp(room_array[m].room,"Bedroom")==0 & strcmp(room_array[m].type,"Heat")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case light_bed2:
          if (strcmp(room_array[m].room,"Bedroom 2") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;            
            output = room_array[m].val.On_Time;
          }
          break;
        case heat_bed2:
          if (strcmp(room_array[m].room,"Bedroom 2") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case heat_bath:
          if (strcmp(room_array[m].room,"Bathroom") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case light_bath:
          if (strcmp(room_array[m].room,"Bathroom") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;

          }
          break;
        case heat_kitchen:
          if (strcmp(room_array[m].room,"Kitchen") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case light_kitchen:
          if (strcmp(room_array[m].room,"Kitchen") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours"); 
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case heat_liv:
          if (strcmp(room_array[m].room,"Living") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case light_liv:
          if (strcmp(room_array[m].room,"Living") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case heat_hall:
          if (strcmp(room_array[m].room,"Hall") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case light_hall:
          if (strcmp(room_array[m].room,"Hall") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
    }
    }else if (pos_state == offTime){
      On_Time_Updated = false;
      Off_Time_Updated = true;
      switch (state){
        case water_garden:
          if (strcmp(room_array[m].room,"Garden")==0 & strcmp(room_array[m].type, "Water")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case light_garage:
          if (strcmp(room_array[m].room,"Garage")==0 & strcmp(room_array[m].type, "Light")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case light_bed1:
          if (strcmp(room_array[m].room,"Bedroom")==0 & strcmp(room_array[m].type, "Light")==0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case heat_bed1:
          if (strcmp(room_array[m].room,"Bedroom") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case light_bed2:
          if (strcmp(room_array[m].room,"Bedroom 2") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case heat_bed2:
          if (strcmp(room_array[m].room,"Bedroom 2") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case heat_bath:
          if (strcmp(room_array[m].room,"Bathroom") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case light_bath:
          if (strcmp(room_array[m].room,"Bathroom") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case heat_kitchen:
          if (strcmp(room_array[m].room,"Kitchen") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case light_kitchen:
          if (strcmp(room_array[m].room,"Kitchen") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case heat_liv:
          if (strcmp(room_array[m].room,"Living") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case light_liv:
          if (strcmp(room_array[m].room,"Living") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case heat_hall:
          if (strcmp(room_array[m].room,"Hall") == 0 & strcmp(room_array[m].type,"Heat") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case light_hall:
          if (strcmp(room_array[m].room,"Hall") == 0 & strcmp(room_array[m].type,"Light") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
      
      }
    }
  }

  hours = changeToTime(output.hours);
  minutes = changeToTime(output.minutes);

  if (first_pass){
    first_pass = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(hours + "." + minutes); 
    lcd.setCursor(0,1);
    lcd.print(F("__"));  
  }

  if (button_state & BUTTON_RIGHT){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(hours + "." + minutes);
    if (timePos == true){
      timePos=false;
      lcd.setCursor(3,1);
      lcd.print(F("__"));
    }else if (timePos == false){
      timePos=true;
      lcd.setCursor(0,1);
      lcd.print(F("__"));
    }
  }else if ((button_state &  BUTTON_UP) || (button_state & BUTTON_DOWN)){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(hours + "." + minutes);
    if (timePos == false){
      timePos=false;
      lcd.setCursor(3,1);
      lcd.print(F("__"));
    }else if (timePos == true){
      timePos=true;
      lcd.setCursor(0,1);
      lcd.print(F("__"));
    }
  }
  return output;
}

timing timeScreenLamp(pos_device state, pos_details pos_state,room_data1 room_array[],int arraySize,int button_state){
  String hours;
  String minutes;
  timing output;
  int m = 0;
  for (m;m<6;m++){
    if (pos_state==onTime){
      On_Time_Updated = true;
      Off_Time_Updated = false;
      //loop through the array to find the correct struct with the correct data
      switch (state){
        case lamp_bed1:
          if (strcmp(room_array[m].room,"Bedroom")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case lamp_bed2:
          if (strcmp(room_array[m].room,"Bedroom 2")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case lamp_bath:
          if (strcmp(room_array[m].room,"Bathroom")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case lamp_kitchen:
          if (strcmp(room_array[m].room,"Kitchen")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        case lamp_liv:
          if (strcmp(room_array[m].room,"Living") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;            
            output = room_array[m].val.On_Time;
          }
          break;
        case lamp_hall:
          if (strcmp(room_array[m].room,"Hall") == 0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.On_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.On_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;
          }
          break;
        }
    }else if (pos_state == offTime){
      On_Time_Updated = false;
      Off_Time_Updated = true;
      switch (state){
        case lamp_bed1:
          if (strcmp(room_array[m].room,"Bedroom")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case lamp_bed2:
          if (strcmp(room_array[m].room,"Bedroom 2")==0){
            if (timePos == true){
              room_array[m].val.On_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");
            } else if (timePos == false) {
              room_array[m].val.On_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.On_Time;           
          }
          break;
        case lamp_bath:
          if (strcmp(room_array[m].room,"Bathroom")==0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case lamp_kitchen:
          if (strcmp(room_array[m].room,"Kitchen") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case lamp_liv:
          if (strcmp(room_array[m].room,"Living") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break;
        case lamp_hall:
          if (strcmp(room_array[m].room,"Hall") == 0){
            if (timePos == true){
              room_array[m].val.Off_Time.hours = changeValue(room_array[m].val.Off_Time.hours,"hours");  
            }else if (timePos == false) {
              room_array[m].val.Off_Time.minutes = changeValue(room_array[m].val.Off_Time.minutes,"minutes");
            }
            valueToChange = m;
            output = room_array[m].val.Off_Time;
          }
          break; 
      }
    }
  }

  hours = changeToTime(output.hours);
  minutes = changeToTime(output.minutes);

  if (first_pass){
    first_pass = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(hours + "." + minutes); 
    lcd.setCursor(0,1);
    lcd.print(F("__"));  
  }

  if (button_state & BUTTON_RIGHT){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(hours + "." + minutes);
    if (timePos == true){
      timePos=false;
      lcd.setCursor(3,1);
      lcd.print(F("__"));
    }else if (timePos == false){
      timePos=true;
      lcd.setCursor(0,1);
      lcd.print(F("__"));
    }
  }else if ((button_state &  BUTTON_UP) || (button_state & BUTTON_DOWN)){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(hours + "." + minutes);
    if (timePos == false){
      timePos=false;
      lcd.setCursor(3,1);
      lcd.print(F("__"));
    }else if (timePos == true){
      timePos=true;
      lcd.setCursor(0,1);
      lcd.print(F("__"));
    }
  }
  return output;
}
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else // __ARM__
extern char *__brkval;
#endif // __arm__

int freeMemory() {
char top;
#ifdef __arm__
return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
return &top - __brkval;
#else // __arm__
return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}

void setup() {

  Serial.begin(9600);
  //if array not present in arduino check for presence
  delay(200);
  int y;
  EEPROM.get(1,y);
  //use 6 as a key to check if data is initiallized in eeprom and itiallize it if not
  if ( y!=6){
    room_data bed1_light = {"Bedroom","Light"};
    room_data bed1_heat = {"Bedroom","Heat"};
    room_data1 bed1_lamp = {"Bedroom","Desk"};

    room_data bed2_light = {"Bedroom 2","Light"};
    room_data bed2_heat = {"Bedroom 2","Heat"};
    room_data1 bed2_lamp = {"Bedroom 2","Desk"};

    room_data kitchen_light = {"Kitchen","Light"};
    room_data kitchen_heat = {"Kitchen","Heat"};
    room_data1 kitchen_lamp = {"Kitchen","Table"};

    room_data bath_light = {"Bathroom","Light"};
    room_data bath_heat = {"Bathroom","Heat"};
    room_data1 bath_lamp = {"Bathroom","Wall"};

    room_data hall_light = {"Hall","Light"};
    room_data hall_heat = {"Hall","Heat"};
    room_data1 hall_lamp = {"Hall","Wall"};

    room_data living_light = {"Living","Light"};
    room_data living_heat = {"Living","Heat"};
    room_data1 living_lamp = {"Living","Table"};

    room_data garden_water = {"Garden","Water"};
    room_data garage_light = {"Garage", "Light"};
    y = 6;
    EEPROM.put(1,y);
    room_data1 lamp_array[6] = {bed1_lamp,bed2_lamp,bath_lamp,kitchen_lamp,hall_lamp,living_lamp};
    room_data room_array2[14] = {bed1_light,bed1_heat,bed2_light,bed2_heat,bath_light,bath_heat,kitchen_light,kitchen_heat,hall_light,hall_heat,living_light,living_heat,garden_water,garage_light};
    EEPROM.put(3,room_array2);
    EEPROM.put(sizeof(room_array2)+3,lamp_array);
    Serial.println(F("ENHANCED:LAMP,OUTSIDE,QUERY,MEMORY,EEPROM"));
  } else {
    Serial.println(F("ENHANCED:LAMP,OUTSIDE,QUERY,MEMORY,EEPROM"));
    Serial.println(F("VALID"));
  }
  lcd.begin(16,2); 
}

void loop() {
  static pos_device cursor1_devBfr = null;
  //buffer to hold previous value of cursor1_dev
  room_data room_array2[14];
  room_data1 lamp_array[6];
  EEPROM.get(3,room_array2);
  EEPROM.get(sizeof(room_array2)+3,lamp_array);
  timing time_output;
  int outputLevel;

  //check for Q in Serial monitor
  if (Serial.available()>0){
    String str = Serial.readString();
    if (str=="Q\n"){
      printall(room_array2,15,lamp_array,7);
    }else if (str=="M\n"){
      Serial.print(F("Bytes of Ram unsued:"));
      Serial.println(freeMemory());
      printall(room_array2,15,lamp_array,7);
    }
  }
  int pressed_buttons = lcd.readButtons();
  //change menu position
  if ((cursor1 == ground)||(cursor1 == first)||(cursor1==details)||(cursor1==outside)){
    slct1("Ground Floor","First Floor","Outside","Print Details",ground,first,outside,details);
  }else if ((cursor1 == bed1)||(cursor1 == bed2)||(cursor1==bath)){
    slct("Bedroom 1","Bedroom 2","Bathroom",bed1,bed2,bath);
  }else if ((cursor1 == kitchen)||(cursor1 == hall)||(cursor1==living)){
    slct("Kitchen","Hall","Living Room",kitchen,hall,living);
  }else if ((cursor1_dev == light_garage)||(cursor1_dev == water_garden)){
    slct5("Garage Light","Garden Water",light_garage,water_garden);
  }else if((cursor1_dev==light_bed1)||(cursor1_dev==heat_bed1)||(cursor1_dev==lamp_bed1)){
    slct2("Light","Heat","Lamp",light_bed1,heat_bed1,lamp_bed1);
  } else if((cursor1_dev==light_bed2)||(cursor1_dev==heat_bed2)||(cursor1_dev==lamp_bed2)){
    slct2("Light","Heat","Lamp",light_bed2,heat_bed2,lamp_bed2);
  }else if((cursor1_dev == light_bath)||(cursor1_dev==heat_bath)||(cursor1_dev==lamp_bath)){
    slct2("Light","Heat","Lamp",light_bath,heat_bath,lamp_bath);
  }else if((cursor1_dev == light_kitchen)||(cursor1_dev==heat_kitchen)||cursor1_dev==lamp_kitchen){
    slct2("Light","Heat","Lamp",light_kitchen,heat_kitchen,lamp_kitchen);
  }else if((cursor1_dev == light_hall)||(cursor1_dev==heat_hall)||(cursor1_dev==lamp_hall)){
    slct2("Light","Heat","Lamp",light_hall,heat_hall,lamp_hall);
  }else if((cursor1_dev == light_liv)||(cursor1_dev==heat_liv)||(cursor1_dev == lamp_liv)){
    slct2("Light","Heat","Lamp",light_liv,heat_liv,lamp_liv);
  }else if(cursor1_dev==null & !value_Screen & cursor1_devBfr != water_garden){
    slct3("On Time","Off Time","Level",onTime,offTime,level);
  }else if (cursor1_devBfr == water_garden & !value_Screen){
    slct4("OnTime","OffTime",onTime,offTime);
  }else if (cursor1_det!=nan){
    if (cursor1_det == level){
      if (cursor1_devBfr == lamp_bath||cursor1_devBfr==lamp_bed1||cursor1_devBfr==lamp_bed2||cursor1_devBfr==lamp_kitchen||cursor1_devBfr==lamp_liv||cursor1_devBfr==lamp_hall){
        outputLevel = levelScreenLamp(cursor1_devBfr,lamp_array,6,pressed_buttons);
        lamp_array[valueToChangeLevel].val.Level = outputLevel;
      }else {
      outputLevel = levelScreen(cursor1_devBfr,room_array2,15,pressed_buttons);
      room_array2[valueToChangeLevel].val.Level = outputLevel;
      }
    }else {
      if (cursor1_devBfr == lamp_bath||cursor1_devBfr==lamp_bed1||cursor1_devBfr==lamp_bed2||cursor1_devBfr==lamp_kitchen||cursor1_devBfr==lamp_liv||cursor1_devBfr==lamp_hall){
        time_output = timeScreenLamp(cursor1_devBfr,cursor1_det,lamp_array,6,pressed_buttons);
        if (On_Time_Updated){
          lamp_array[valueToChange].val.On_Time = time_output;
        } else if (Off_Time_Updated){
          lamp_array[valueToChange].val.Off_Time = time_output;
        }
      }else{
        time_output = timeScreen(cursor1_devBfr,cursor1_det,room_array2,15,pressed_buttons);
        if (On_Time_Updated){
          room_array2[valueToChange].val.On_Time = time_output;
        } else if (Off_Time_Updated){
          room_array2[valueToChange].val.Off_Time = time_output;
        }
      }
    }
    EEPROM.put(sizeof(room_array2)+3,lamp_array);
    EEPROM.put(3,room_array2);
  }

  //when select is pressed check where in the menu to move to and move there
  if (pressed_buttons & BUTTON_SELECT){
    if (cursor1_det==nan){
      switch (cursor1){
        case non:
          if (cursor1_dev == water_garden){
          cursor1_devBfr = cursor1_dev;
          cursor1_dev = null;
          first_pass = true;
          cursor1_det = onTime;
          slct4("OnTime","OffTime",onTime,offTime);
          }
          else{
          cursor1_devBfr = cursor1_dev;
          cursor1_dev = null;
          first_pass = true;
          cursor1_det = onTime;
          slct3("On Time","Off Time","Level",onTime,offTime,level);
          }
          break;      
        case first:
          first_pass = true;
          cursor1 = bed1;
          slct("Bedroom 1","Bedroom 2","Bathroom",bed1,bed2,bath);
          break;
        case ground:
          first_pass = true;
          cursor1 = kitchen;
          slct("Kitchen","Hall","Living Room",kitchen,hall,living);
          break;
        case outside:
          changeScreen1(light_garage,water_garden,Outside,"Garage Light","Garden Water",non);
          break;
        case details:
          printall(room_array2,15,lamp_array,7);
          break;
        case bed1:
          changeScreen(light_bed1,heat_bed1,lamp_bed1,bed_room1,"Light","Heat","Lamp",non);
          break;
        case bed2:
          changeScreen(light_bed2,heat_bed2,lamp_bed2,bed_room2,"Light","Heat","Lamp",non);
          break;
        case bath:
          changeScreen(light_bath,heat_bath,lamp_bath,bath_room,"Light","Heat","Lamp",non);
          break;
        case kitchen:
          changeScreen(light_kitchen,heat_kitchen,lamp_kitchen,kitchen_room,"Light","Heat","Lamp",non);
          break;
        case hall:
          changeScreen(light_hall,heat_hall,lamp_hall,hall_room,"Light","Heat","Lamp",non);
          break;
        case living:
          changeScreen(light_liv,heat_liv,lamp_liv,liv_room,"Light","Heat","Lamp",non);
          break;  
      }
    } else{
        if (cursor1_det == level){
          first_pass = true;
          if (cursor1_devBfr == lamp_bath||cursor1_devBfr==lamp_bed1||cursor1_devBfr==lamp_bed2||cursor1_devBfr==lamp_kitchen||cursor1_devBfr==lamp_liv||cursor1_devBfr==lamp_hall){
            outputLevel = levelScreenLamp(cursor1_devBfr,lamp_array,6,pressed_buttons);
          }else{
            outputLevel = levelScreen(cursor1_devBfr,room_array2,15,pressed_buttons);
          }
          value_Screen = true;
        }else{
          first_pass = true;
          if (cursor1_devBfr == lamp_bath||cursor1_devBfr==lamp_bed1||cursor1_devBfr==lamp_bed2||cursor1_devBfr==lamp_kitchen||cursor1_devBfr==lamp_liv||cursor1_devBfr==lamp_hall){
            time_output = timeScreenLamp(cursor1_devBfr,cursor1_det,lamp_array,6,pressed_buttons);
          }else{
            time_output = timeScreen(cursor1_devBfr,cursor1_det,room_array2,15,pressed_buttons);
          }
          value_Screen = true;
        }
    }
  }
  //on left press (go to previous menu)
  if (pressed_buttons & BUTTON_LEFT){
    first_pass = true;
    if (cursor1 == bed1 || cursor1 == bed2 || cursor1==bath){
      cursor1 = ground;
    } else if ((cursor1 == kitchen)||(cursor1 == hall)||(cursor1==living)) {
    cursor1 = ground;
    }else if(((room==bed_room1)||(room==bed_room2)||(room==bath_room)) & (cursor1_det==nan)){
      room = nun;
      cursor1=bed1;
    }else if (((room==kitchen_room)||(room==hall_room)||(room==liv_room)) & (cursor1_det==nan)){
      room = nun;
      cursor1=kitchen;
    }else if (room==Outside & cursor1_det==nan){
      room = nun;
      cursor1 = ground;
    }
    if (cursor1_det!=nan){
      cursor1_det = nan;
      value_Screen = false; 
        if (room==bed_room1){
          cursor1_dev = light_bed1;
        } else if (room==bed_room2){
          cursor1_dev = light_bed2;
        } else if(room==bath_room){
          cursor1_dev = light_bath;
        }else if (room==kitchen_room){
          cursor1_dev = light_kitchen;
        }else if(room==hall_room){
          cursor1_dev = light_hall;
        } else if(room==liv_room){
          cursor1_dev = light_liv;
        } else if (room==Outside){
          cursor1_dev = light_garage;
      }
    }
  }
  delay(100);
}
