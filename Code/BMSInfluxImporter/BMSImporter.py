import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
import csv 
from datetime import datetime

bucket = "CHANGE_ME"
org = "CHANGE_ME"
token = "CHANGE_ME"
url="CHANGE_ME"

pointseries_name = "netzero_queens_test"

client = influxdb_client.InfluxDBClient(
    url=url,
    token=token,
    org=org
)

def insertCSV(OAT_CSVFile, topic):
    with open(OAT_CSVFile) as file_obj: 
     
        write_api = client.write_api(write_options=SYNCHRONOUS)
        
        # Skips the heading 
        # Using next() method 
        heading = next(file_obj) 
          
        # Create reader object by passing the file  
        # object to reader method 
        reader_obj = csv.reader(file_obj) 
          
        # Iterate over each row in the csv file   03/02/24 11:43:37 GMT
        for row in reader_obj:
            date = datetime.strptime(row[0].replace(" GMT", ""), "%d/%m/%y %H:%M:%S")
            
            p = influxdb_client.Point(pointseries_name).tag("topic", topic).field("value", float(row[1])).time(date)
            write_api.write(bucket=bucket, org=org, record=p)
            print(date) 

## Data Part 1
insertCSV('datasets1\BoilerFlow_celsius.csv', "project/NetZero/Queens/theatre/4/BoilerFlow_celsius")
insertCSV('datasets1\BoilerReturn_celsius.csv', "project/NetZero/Queens/theatre/4/BoilerReturn_celsius")
insertCSV('datasets1\ChillerFlow_celsius.csv', "project/NetZero/Queens/theatre/4/ChillerFlow_celsius")
insertCSV('datasets1\ChillerReturn_celsius.csv', "project/NetZero/Queens/theatre/4/ChillerReturn_celsius")
insertCSV('datasets1\OATemp_celsius.csv', "project/NetZero/Queens/theatre/4/OATemp_celsius")
insertCSV('datasets1\ReturnAirTemp_celsius.csv', "project/NetZero/Queens/theatre/4/ReturnAirTemp_celsius")

insertCSV('datasets1\CoolingValve_percent.csv', "project/NetZero/Queens/theatre/4/CoolingValve_percent")
insertCSV('datasets1\FrostValve_percent.csv', "project/NetZero/Queens/theatre/4/FrostValve_percent")
insertCSV('datasets1\HeatingValve_percent.csv', "project/NetZero/Queens/theatre/4/HeatingValve_percent")

insertCSV('datasets1\RoomHumidity_RH.csv', "project/NetZero/Queens/theatre/4/RoomHumidity_RH")
insertCSV('datasets1\RunAroundPmpCMD_OnOff.csv', "project/NetZero/Queens/theatre/4/RunAroundPmpCMD_OnOff")
insertCSV('datasets1\Setback_OnOff.csv', "project/NetZero/Queens/theatre/4/Setback_OnOff")




## Data Part 2
insertCSV('datasets2\BoilerFlow_celsius.csv', "project/NetZero/Queens/theatre/4/BoilerFlow_celsius")
insertCSV('datasets2\BoilerReturn_celsius.csv', "project/NetZero/Queens/theatre/4/BoilerReturn_celsius")
insertCSV('datasets2\ChillerFlow_celsius.csv', "project/NetZero/Queens/theatre/4/ChillerFlow_celsius")
insertCSV('datasets2\ChillerReturn_celsius.csv', "project/NetZero/Queens/theatre/4/ChillerReturn_celsius")
insertCSV('datasets2\OATemp_celsius.csv', "project/NetZero/Queens/theatre/4/OATemp_celsius")
insertCSV('datasets2\ReturnAirTemp_celsius.csv', "project/NetZero/Queens/theatre/4/ReturnAirTemp_celsius")

insertCSV('datasets2\CoolingValve_percent.csv', "project/NetZero/Queens/theatre/4/CoolingValve_percent")
insertCSV('datasets2\FrostValve_percent.csv', "project/NetZero/Queens/theatre/4/FrostValve_percent")
insertCSV('datasets2\HeatingValve_percent.csv', "project/NetZero/Queens/theatre/4/HeatingValve_percent")

insertCSV('datasets2\RoomHumidity_RH.csv', "project/NetZero/Queens/theatre/4/RoomHumidity_RH")
insertCSV('datasets2\RoomTemperature_celsius.csv', "project/NetZero/Queens/theatre/4/RoomTemperature_celsius")

