if [ -e ./runtest.sh ]; then
  . runtest.sh
elif [ -e tests/runtest.sh ]; then
  cd tests
  . runtest.sh
else
  echo wrong PWD
  exit 1
fi
