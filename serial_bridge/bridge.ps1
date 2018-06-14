$jevois_port_name  = COM3
$arduino_port_name = COM4

$jevois_port = new-Object System.IO.Ports.SerialPort $jevois_port_name,115200,None,8,one
$arduino_port = new-Object System.IO.Ports.SerialPort $arduino_port_name,115200,None,8,one

$jevois_port.ReadTimeout = 50
$arduino_port.ReadTimeout = 50

echo "[bridge] Starting JeVois"

$jevois_port.write("GRATERTTT")

echo "[bridge] Starting serial listening. Press Q to quit."


while($true){

    try{
        $jevois_output  = $jevois_port.ReadLine()
        $arduino_output = $arduino_port.ReadLine()
        echo $jevois_output
        echo $arduino_port
        $arduino_port.write($jevois_output)
        
    } catch {
       # Start-Sleep -m 50
    }

    # Check if Q was pressed and quit if so.
    if ([console]::KeyAvailable) {
        $key = [system.console]::readkey($true)
        if (($key.key -eq "Q")) {
            echo "Quitting, user pressed Q..."
            break
        }
    }
    
}