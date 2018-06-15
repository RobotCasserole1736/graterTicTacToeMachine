$jevois_port_name  = "COM6"
$arduino_port_name = "COM4"

$jevois_port = new-Object System.IO.Ports.SerialPort $jevois_port_name,115200,None,8,one
$arduino_port = new-Object System.IO.Ports.SerialPort $arduino_port_name,115200,None,8,one

$jevois_port.Open()
$arduino_port.Open()

$jevois_port.ReadTimeout = 200
$arduino_port.ReadTimeout = 200

echo "[bridge] Starting serial listening. Press Q to quit."


while($true){

    try{
        $jevois_output  = $jevois_port.ReadLine()
		echo $jevois_output
        $arduino_port.write($jevois_output)
    } catch {
       # Start-Sleep -m 50
    }
	
	try{
        $arduino_output = $arduino_port.ReadLine()    
        echo $arduino_output
    } catch {
       # Start-Sleep -m 50
    }

    # Check if Q was pressed and quit if so.
    if ([console]::KeyAvailable) {
        $key = [system.console]::readkey($true)
        if (($key.key -eq "Q")) {
            echo "[bridge] Quitting, user pressed Q..."
            break
        } elseif  (($key.key -eq "g")) {
            echo "[bridge] Sending start command"
            $arduino_port.write("g")
			$jevois_port.write("GRATERTTM")
        }
    }
    
}


$jevois_port.Close()
$arduino_port.Close()