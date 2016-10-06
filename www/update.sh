#/bin/bash

PORT=${1:-1223}

scp -r -P $PORT ../www localhost:
ssh -p $PORT localhost << 'ENDSSH'
	set -x
	sudo rm -rf /usr/share/nginx/html/*
	sudo mv /home/vmaffione/www/* /usr/share/nginx/html/
	sudo truncate --size 500MB /usr/share/nginx/html/library.tar.gz
ENDSSH
