param($Port="COM11", $HexFile="build\blink.hex", $Baud=9600)
$crcT=@();for($i=0;$i -lt 256;$i++){$c=$i;for($j=0;$j -lt 8;$j++){$c=$(&{if($c -band 1){($c -shr 1)-bxor 0xEDB88320}else{$c -shr 1}})};$crcT+=$c}
function CRC32([byte[]]$d){$c=0xFFFFFFFF;foreach($b in $d){$c=($c -shr 8)-bxor $crcT[($c -bxor $b)-band 0xFF]};return $c -bxor 0xFFFFFFFF}
function BP([byte]$cmd,[byte[]]$pl){$len=1;if($pl){$len+=$pl.Length};$p=[byte[]]::new(4+$(if($pl){$pl.Length}else{0})+4);$p[0]=0x80;$p[1]=[byte]($len -band 0xFF);$p[2]=[byte](($len -shr 8)-band 0xFF);$p[3]=$cmd;$c=[byte[]]::new($len);$c[0]=$cmd;if($pl){[Array]::Copy($pl,0,$c,1,$pl.Length)};$crc=CRC32 $c;$off=4+$(if($pl){$pl.Length}else{0});$p[$off]=[byte]($crc -band 0xFF);$p[$off+1]=[byte](($crc -shr 8)-band 0xFF);$p[$off+2]=[byte](($crc -shr 16)-band 0xFF);$p[$off+3]=[byte](($crc -shr 24)-band 0xFF);return $p}
function WP([System.IO.Ports.SerialPort]$p,[byte[]]$pk){$p.Write($pk,0,$pk.Length);$a=0;try{$a=$p.ReadByte()}catch{};return $a}

$port=New-Object System.IO.Ports.SerialPort($Port,$Baud,'None',8,'One')
$port.ReadTimeout=5000;$port.WriteTimeout=5000
$port.Open()
Write-Host "Hold BSL+RST!";Start-Sleep 4

# Sync
$port.ReadExisting()|Out-Null
for($i=0;$i -lt 20;$i++){$port.Write(@(0x55),0,1);Start-Sleep -m 20}
$r=0;try{$r=$port.ReadByte()}catch{};Write-Host "Sync: $r"
$port.Write(@(0xBB),0,1);Start-Sleep -m 600

# Connect
$ack=WP $port (BP 0x12 $null);if($ack -ne 0){$port.Close();exit 1}
Write-Host "Connected"

# Unlock
$pwd=[byte[]]::new(32);for($i=0;$i -lt 32;$i++){$pwd[$i]=0xFF}
$ack=WP $port (BP 0x21 $pwd);$port.Read([byte[]]::new(5),0,5)|Out-Null;Write-Host "Unlock: $ack"

# Erase
$ack=WP $port (BP 0x15 $null);Start-Sleep 3;$port.Read([byte[]]::new(5),0,5)|Out-Null;Write-Host "Erase: $ack"

# Parse hex
$seg=@{};$ba=0;Get-Content $HexFile|%{if($_ -match '^:(\w\w)(\w\w\w\w)(\w\w)(\w+)(\w\w)$'){$t=[Convert]::ToInt32($Matches[3],16);$a=[Convert]::ToInt32($Matches[2],16);$d=$Matches[4];if($t -eq 0){for($i=0;$i -lt $d.Length;$i+=2){$seg[$ba+$a+$i/2]=[byte][Convert]::ToInt32($d.Substring($i,2),16)}}elseif($t -eq 4){$ba=[Convert]::ToInt32($d,16)*65536}}}
$ks=@($seg.Keys|Sort);$s=$ks[0];$pv=$s;$bf=[System.Collections.ArrayList]::new();$bl=@()
for($i=0;$i -lt $ks.Count;$i++){$k=$ks[$i];if($k -gt $pv+1 -and $bf.Count -gt 0){$bl+=,[PSCustomObject]@{A=$s;D=$bf.ToArray()};$s=$k;$bf=[System.Collections.ArrayList]::new()};[void]$bf.Add($seg[$k]);$pv=$k}
$bl+=,[PSCustomObject]@{A=$s;D=$bf.ToArray()}

# Write
foreach($b in $bl){$addr=$b.A;$data=$b.D;for($o=0;$o -lt $data.Length;$o+=128){$n=[Math]::Min(128,$data.Length-$o);$ch=[byte[]]::new($n);[Array]::Copy($data,$o,$ch,0,$n);$pl=[byte[]]::new(4+$n);$pl[0]=[byte]($addr -band 0xFF);$pl[1]=[byte](($addr -shr 8)-band 0xFF);$pl[2]=[byte](($addr -shr 16)-band 0xFF);$pl[3]=[byte](($addr -shr 24)-band 0xFF);[Array]::Copy($ch,0,$pl,4,$n);$ack=WP $port (BP 0x20 $pl);Write-Host "W 0x$($addr.ToString('X8')) +$n B: $ack";$addr+=$n}}

# Start
$ack=WP $port (BP 0x40 $null);$port.Close()
Write-Host "✅ DONE - Press RST"
