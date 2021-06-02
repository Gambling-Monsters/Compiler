path="../Test/"
files=$(ls $path)
for filename in $files
do
   echo -e "\n"$filename >> filename.txt
   ./parser $path$filename >> filename.txt
done