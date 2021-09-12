#!/bin/bash
trap "exit" INT
if ! [[ ( $# == 1 ) || ( $# == 2 ) ]];
    then echo "Invocation:\n$0 <Directory> <Output Root>\n\n"
    exit
fi
if ! [[ -d $1 ]];
    then echo "$1 is not a directory"
    exit
fi
for file in $(find $1 -maxdepth 1 -type f); do
    if [[ $file == *.cat ]] # catalog file found
    then 
        echo "Catalog file found: $file"
        rootdir=$(dirname $file)
        basedir="${rootdir%"${rootdir##*[!/]}"}" # extglob-free multi-trailing-/ trim
        basedir="${basedir##*/}"                  # remove everything before the last /
        echo $basedir
        remotedir=basedir
        if [ $# == 2 ];
            then
            remotedir="${2}/${basedir}_fits"
            echo "Creating directory: $remotedir"
        else
            remotedir="${PWD}/${basedir}_fit"
            echo $remotedir
        fi
        mkdir -p $remotedir
        cp $file $remotedir
        while IFS= read -r line
        do
            filename=$(echo $line | awk '{print $1}')
            ./convert.out $1/$filename
            mv temp.fit $remotedir/$filename.fit
        done < "$file"
    fi
done;
