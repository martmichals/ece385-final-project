#!/bin/bash

USERNAME=ubuntu
IP=149.56.141.231
DEPLOYPATH="~/server/"

echo "Uploading files..."
scp *.js $USERNAME@$IP:$DEPLOYPATH
scp package.json $USERNAME@$IP:$DEPLOYPATH
scp package-lock.json $USERNAME@$IP:$DEPLOYPATH

echo "Files have been uploaded."
