@INCLUDE_COMMON@

echo
echo ELEKTRA BASIC SCRIPTS TESTS
echo

check_version


OURS_ROOT=$USER_ROOT/mergetest/ours
THEIRS_ROOT=$USER_ROOT/mergetest/theirs
BASE_ROOT=$USER_ROOT/mergetest/base
MERGED_ROOT=$USER_ROOT/mergetest/merged

echo "Testing help"

$KDB -H >/dev/null
exit_if_fail "could not get help"

$KDB --help >/dev/null
exit_if_fail "could not get help"

$KDB -V | grep KDB_VERSION | grep $KDB_VERSION > /dev/null
exit_if_fail "could not get correct version"


echo "Testing version"

$KDB --version | grep KDB_VERSION | grep $KDB_VERSION > /dev/null
exit_if_fail "could not get correct version"

$KDB -V | grep KDB_VERSION | grep $KDB_VERSION > /dev/null
exit_if_fail "could not get correct version"

$KDB --version | grep KDB_VERSION | grep $KDB_VERSION > /dev/null
exit_if_fail "could not get correct version"


echo "Testing invalid"

$KDB ls --ww >/dev/null
[ $? = 1 ]
exit_if_fail "invalid options"

$KDB ls x x x x  >/dev/null
[ $? = 2 ]
exit_if_fail "invalid argument"

$KDB xx  >/dev/null
[ $? = 4 ]
exit_if_fail "invalid command"

end_script
