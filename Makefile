

test:
	tr -d ' \n' < bytes.hex | xxd -r -p > test.bin
