#!/bin/sh                                                                       
                                                                                
SERVER="http://192.168.7.1:8000"                                                
                                                                                
TMP_BIN="/tmp/sensor_app"                                                       
TMP_VER="/tmp/version.txt"                                                      
TMP_CFG="/tmp/sensor_app.conf"                                                  
TMP_SHA="/tmp/sensor_app.sha256"                                                
                                                                                
APP_BIN="/usr/bin/sensor_app"                                                   
APP_CFG="/etc/sensor_app.conf"                                                  
APP_VER_FILE="/etc/sensor_app.version"                                          
                                                                                
BACKUP_BIN="/usr/bin/sensor_app.bak"                                            
BACKUP_CFG="/etc/sensor_app.conf.bak"                                           
BACKUP_VER="/etc/sensor_app.version.bak"                                        
                                                                                
echo "Checking for OTA update..."                                               
                                                                                
if [ -f "$APP_VER_FILE" ]; then                                                 
    CURRENT_VER=$(cat "$APP_VER_FILE" | tr -d '\r\n')                           
else                                                                            
    CURRENT_VER="unknown"                                                       
fi                                                                              
                                                                                
wget -T 5 -t 1 -O "$TMP_VER" "$SERVER/version.txt"                              
if [ $? -ne 0 ]; then                                                           
    echo "Failed to download version file"                                      
    exit 1                                                                      
fi                                                                              
                                                                                
NEW_VER=$(cat "$TMP_VER" | tr -d '\r\n')                                        
                                                                                
echo "Current version: $CURRENT_VER"                                            
echo "Available version: $NEW_VER"                                              
                                                                                
if [ "$CURRENT_VER" = "$NEW_VER" ]; then                                        
    echo "Already up to date"                                                   
    exit 0                                                                      
fi                                                                              
                                                                                
echo "Downloading new binary..."                                                
wget -T 5 -t 1 -O "$TMP_BIN" "$SERVER/sensor_app"                               
if [ $? -ne 0 ]; then                                                           
    echo "Failed to download new binary"                                        
    exit 1                                                                      
fi                                                                              
                                                                                
echo "Downloading checksum..."                                                  
wget -T 5 -t 1 -O "$TMP_SHA" "$SERVER/sensor_app.sha256"                        
if [ $? -ne 0 ]; then                                                           
    echo "Failed to download checksum"                                          
    exit 1                                                                      
fi                                                                              
                                                                                
cd /tmp || exit 1                                                               
sha256sum -c "$TMP_SHA"                                                         
if [ $? -ne 0 ]; then                                                           
    echo "Checksum verification failed"                                         
    exit 1                                                                      
fi                                                                              
                                                                                
echo "Downloading new config..."                                                
wget -T 5 -t 1 -O "$TMP_CFG" "$SERVER/sensor_app.conf"                          
if [ $? -ne 0 ]; then                                                           
    echo "Failed to download new config"                                        
    exit 1                                                                      
fi                                                                              
                                                                                
chmod +x "$TMP_BIN"                                                             
                                                                                
echo "Stopping service..."                                                      
systemctl stop sensor_app.service                                               
                                                                                
[ -f "$APP_BIN" ] && cp "$APP_BIN" "$BACKUP_BIN"                                
[ -f "$APP_CFG" ] && cp "$APP_CFG" "$BACKUP_CFG"                                
[ -f "$APP_VER_FILE" ] && cp "$APP_VER_FILE" "$BACKUP_VER"                      
                                                                                
mv "$TMP_BIN" "$APP_BIN"                                                        
chmod +x "$APP_BIN"                                                             
                                                                                
mv "$TMP_CFG" "$APP_CFG"                                                        
chmod 644 "$APP_CFG"                                                            
                                                                                
echo "$NEW_VER" > "$APP_VER_FILE"                                               
chmod 644 "$APP_VER_FILE"                                                       
                                                                                
echo "Starting service..."                                                      
systemctl start sensor_app.service                                              
sleep 2                                                                         
                                                                                
systemctl is-active --quiet sensor_app.service                                  
if [ $? -ne 0 ]; then                                                           
    echo "Update failed, rolling back..."                                       
                                                                                
    [ -f "$BACKUP_BIN" ] && cp "$BACKUP_BIN" "$APP_BIN" && chmod +x "$APP_BIN"  
    [ -f "$BACKUP_CFG" ] && cp "$BACKUP_CFG" "$APP_CFG" && chmod 644 "$APP_CFG" 
    [ -f "$BACKUP_VER" ] && cp "$BACKUP_VER" "$APP_VER_FILE" && chmod 644 "$APP"
                                                                                
    systemctl start sensor_app.service                                          
    exit 1                                                                      
fi                                                                              
                                                                                
echo "OTA update successful"
