read -p "Enter Hostname/IP: " myhost
read -p "Enter Username: " myuser
cat ~/.ssh/id_rsa.pub | ssh $myuser@$myhost 'mkdir .ssh -p; cat >> .ssh/authorized_keys'
