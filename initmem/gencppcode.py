import sys

def main():
	if len(sys.argv) < 3:
		print 'usage ' + sys.argv[0] + ' FCOUNT VCOUNT'
	
	prologue = "#include<iostream>\n\nusing namespace std;\n"
	epilogue = "\nreturn 0;\n}"
	code = prologue
	for f in range(1,int(sys.argv[1])):
		code += '\nint foo' + str(f) + "();"

	for f in range(int(sys.argv[1])):
		if f == 0:
			code += "\n\nint main() {"
			code += get_variables("m", int(sys.argv[2]))
			for f in range(1,int(sys.argv[1])):
				code += '\nfoo' + str(f) + "();"
		else:
			code += "\n\nint foo" + str(f) + "(){"
			code += get_variables("f"+str(f), int(sys.argv[2]))
		code += epilogue
	with open("sample_test.cc", "w") as f:
		f.write(code)

def get_variables(suf, var):
	vcode = ""
	suf +=  "var"
	for v in range(var):
		fvar = "\n\tint "+ suf + str(v) + " ;"
		vcode += fvar
	for v in range(var):
		prvar = '\n\tcout << "'+ suf + str(v) +' = " << '+ suf + str(v) + ' << "\\n";'
		vcode += prvar
	return vcode

if __name__ == "__main__":
	main()
