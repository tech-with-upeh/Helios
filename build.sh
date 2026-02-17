cmake --build build --preset conan-release

mv -f build/build/Release/helios examples/helios/

echo "done Building Boss."