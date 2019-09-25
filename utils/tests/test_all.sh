dirname=`dirname $0`
echo "* Run all tests in $dirname/*"
python -m unittest discover $dirname
