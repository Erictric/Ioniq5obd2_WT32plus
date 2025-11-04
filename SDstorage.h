//----------------------------------------------------------------------------------------
//        Task on core 0 to Send data to Google Sheet via IFTTT web service Function                                            
//----------------------------------------------------------------------------------------

void makeIFTTTRequest(void * pvParameters){
  for(;;){
    if (send_enabled && send_data) {
      code_sent = false;
      Serial.print("Connecting to "); 
      Serial.print(server);
      
      WiFiClient client;
      int retries = 5;
      while(!!!client.connect(server, 80) && (retries-- > 0)) {
        Serial.print(".");
      }
      Serial.println();
      if(!!!client.connected()) {
        Serial.println("Failed to connect...");
        code_sent = true;
      }
      
      Serial.print("Request resource: "); 
      Serial.println(resource);
      
      float sensor_Values[nbParam];
      
      char column_name[ ][15]={"SoC","Power","BattMinT","Heater","Net_Ah","Net_kWh","AuxBattSoC","AuxBattV","Max_Pwr","Max_Reg","BmsSoC","MAXcellv","MINcellv","MAXcellvNb","MINcellvNb","BATTv","BATTc","Speed","Odometer","CEC","CED","CDC","CCC","SOH","BMS_ign","OPtimemins","OUTDOORtemp","INDOORtemp","SpdSelect","LastSoC","Calc_Used","Calc_Left","TripOPtime","CurrOPtime","PIDkWh_100","kWh_100km","degrad_ratio","EstLeft_kWh","span_kWh_100km","SoCratio","nbr_powerOn","TireFL_P","TireFR_P","TireRL_P","TireRR_P","TireFL_T","TireFR_T","TireRL_T","TireRR_T","acc_energy","Trip_dist","distance","BattMaxT","acc_Ah","acc_kWh_25","acc_kWh_10","acc_kWh_0","acc_kWh_m10","acc_kWh_m20","acc_kWh_m20p","acc_time_25","acc_time_10","acc_time_0","acc_time_m10","acc_time_m20","acc_time_m20p","acc_dist_25","acc_dist_10","acc_dist_0","acc_dist_m10","acc_dist_m20","acc_dist_m20p","acc_regen","MaxDetNb","MinDetNb","Deter_Min"};;
      
      sensor_Values[0] = SoC;
      sensor_Values[1] = Power;
      sensor_Values[2] = BattMinT;
      sensor_Values[3] = Heater;
      sensor_Values[4] = Net_Ah;
      sensor_Values[5] = Net_kWh;
      sensor_Values[6] = AuxBattSoC;
      sensor_Values[7] = AuxBattV;
      sensor_Values[8] = Max_Pwr;
      sensor_Values[9] = Max_Reg;  
      sensor_Values[10] = BmsSoC;
      sensor_Values[11] = MAXcellv;
      sensor_Values[12] = MINcellv;
      sensor_Values[13] = MAXcellvNb;
      sensor_Values[14] = MINcellvNb;
      sensor_Values[15] = BATTv;
      sensor_Values[16] = BATTc;
      sensor_Values[17] = Speed;
      sensor_Values[18] = Odometer;
      sensor_Values[19] = CEC;
      sensor_Values[20] = CED;
      sensor_Values[21] = CDC;
      sensor_Values[22] = CCC;
      sensor_Values[23] = SOH;
      sensor_Values[24] = BMS_ign;        
      sensor_Values[25] = OPtimemins;
      sensor_Values[26] = OUTDOORtemp;
      sensor_Values[27] = INDOORtemp;
      sensor_Values[28] = SpdSelect;
      sensor_Values[29] = LastSoC;
      sensor_Values[30] = used_kwh;
      sensor_Values[31] = left_kwh;
      sensor_Values[32] = TripOPtime;
      sensor_Values[33] = CurrOPtime;      
      sensor_Values[34] = PIDkWh_100;
      sensor_Values[35] = kWh_100km;
      sensor_Values[36] = degrad_ratio;
      sensor_Values[37] = EstLeft_kWh;
      sensor_Values[38] = span_kWh_100km;
      sensor_Values[39] = SoCratio;
      sensor_Values[40] = nbr_powerOn;
      sensor_Values[41] = TireFL_P;
      sensor_Values[42] = TireFR_P;
      sensor_Values[43] = TireRL_P;
      sensor_Values[44] = TireRR_P;
      sensor_Values[45] = TireFL_T;
      sensor_Values[46] = TireFR_T;
      sensor_Values[47] = TireRL_T;
      sensor_Values[48] = TireRR_T;
      sensor_Values[49] = acc_energy;
      sensor_Values[50] = Trip_dist;
      sensor_Values[51] = distance; 
      sensor_Values[52] = BattMaxT;
      sensor_Values[53] = acc_Ah;
      sensor_Values[54] = acc_kWh_25;
      sensor_Values[55] = acc_kWh_10;
      sensor_Values[56] = acc_kWh_0;
      sensor_Values[57] = acc_kWh_m10;
      sensor_Values[58] = acc_kWh_m20;
      sensor_Values[59] = acc_kWh_m20p;
      sensor_Values[60] = acc_time_25;
      sensor_Values[61] = acc_time_10;
      sensor_Values[62] = acc_time_0;
      sensor_Values[63] = acc_time_m10;
      sensor_Values[64] = acc_time_m20;
      sensor_Values[65] = acc_time_m20p;
      sensor_Values[66] = acc_dist_25;
      sensor_Values[67] = acc_dist_10;
      sensor_Values[68] = acc_dist_0;
      sensor_Values[69] = acc_dist_m10;
      sensor_Values[70] = acc_dist_m20;
      sensor_Values[71] = acc_dist_m20p;
      sensor_Values[72] = acc_regen;
      sensor_Values[73] = MaxDetNb;
      sensor_Values[74] = MinDetNb;
      sensor_Values[75] = Deter_Min;
      
      String headerNames = "";
      String payload ="";
      
      int i=0;
      
      if(initscan || record_code != 0 || shutdown_esp){
        switch (record_code)
        {
            case 0:   // No reset only header required, ESP32 power reboot
              while(i!=nbParam)
              {
                if(i==0){
                  headerNames = String("{\"value1\":\"") + column_name[i];
                  i++;
                }
                if(i==nbParam)
                  break;
                headerNames = headerNames + "|||" + column_name[i];
                i++;    
              }
              initscan = false;
              break;

            case 1:   // Write status for Reset after a battery was recharged
              headerNames = String("{\"value1\":\"") + "|||" + "Battery_Recharged" + "|||" + "LastSoc:" + "|||" + mem_LastSoC + "|||" + "Soc:" + "|||" + mem_SoC + "|||" + "Power:" + "|||"  + mem_Power;
              record_code = 0;
              initscan = true;
              break;

            case 2:   // Write status for Reset performed with reset button (right button)
              headerNames = String("{\"value1\":\"") + "|||" + "Button_Reset";
              record_code = 0;
              initscan = true;
              break;

            case 3:   // Write status for Reset when Acc_energy is less then 0.3kWh when SoC changes
              headerNames = String("{\"value1\":\"") + "|||" + "ACC_energy <0.3" + "|||" + "acc_energy:" + "|||" + mem_energy + "|||" + "PreSoC:" + "|||" + mem_PrevSoC + "|||" + "SoC:" + "|||" + mem_SoC;
              record_code = 0;
              initscan = true;
              break;

            case 4:   // Write status for Reset if SoC changes from 100 to 99% not going through 99.5%
              headerNames = String("{\"value1\":\"") + "|||" + "100_to_99SoC_reset" + "|||" + "PreSoc:" + "|||" + mem_PrevSoC + "|||" + "SoC:" + "|||" + mem_SoC;
              record_code = 0;
              initscan = true;
              break;

            case 5:   // Write that esp is going normal shutdown
            headerNames = String("{\"value1\":\"") + "|||" + "Normal Shutdown" + "|||" + "Power:" + "|||" + Power + "|||" + "SoC:" + "|||" + mem_SoC;            
            code_received = true;            
            Serial.println("Code Received");
            break;

            case 6:   // Write that esp is going timed shutdown
            headerNames = String("{\"value1\":\"") + "|||" + "Timer Shutdown" + "|||" + "Power:" + "|||" + Power + "|||" + "Timer:" + "|||" + shutdown_timer;            
            code_received = true;            
            break;

            case 7:   // Write that esp is going low 12V shutdown
            headerNames = String("{\"value1\":\"") + "|||" + "Low 12V Shutdown" + "|||" + "12V Batt.:" + "|||" + AuxBattSoC + "|||" + "Timer:" + "|||" + shutdown_timer;            
            code_received = true;            
            break;

        }      
          payload = headerNames;
      }
        
      else{
        while(i!=nbParam) 
        {
          if(i==0)
          {
            payload = String("{\"value1\":\"") + sensor_Values[i];
            i++;
          }
          if(i==nbParam)
          {
             break;
          }
          payload = payload + "|||" + sensor_Values[i];
          i++;    
        }
      }      
      
      String jsonObject = payload + "\"}";                          
                       
      client.println(String("POST ") + resource + " HTTP/1.1");
      client.println(String("Host: ") + server); 
      client.println("Connection: close\r\nContent-Type: application/json");
      client.print("Content-Length: ");
      client.println(jsonObject.length());
      client.println();
      client.println(jsonObject);
                  
      int timeout = 5; // 50 * 100mS = 5 seconds            
      while(!!!client.available() && (timeout-- > 0)){
        delay(100);
      }
      if(!!!client.available()) {
        Serial.println("No response...");
        code_sent = true;
      }
      while(client.available()){
        Serial.write(client.read());
      }
      
      Serial.println();
      Serial.println("closing connection");
      client.stop();
          
      send_data = false;
      pwr_changed = 0;
      loop_count = 0;
      
      if(kWh_update){ //add condition so "kWh_corr" is not trigger before a cycle after a "kWh_update"
        Prev_kWh = Net_kWh;        
        kWh_update = false;  // reset kWh_update after it has been recorded and so the correction logic start again       
      }            
      if(corr_update){  
        corr_update = false;  // reset corr_update after it has been recorded
      }
      if (code_received){
        Serial.println("Sending code sent");
        code_sent = true;
      }      
    }    
    vTaskDelay(10); // some delay is required to reset watchdog timer
  }
}