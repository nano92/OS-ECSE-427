def Main():
	fo = open("text.txt","w")
	fo.write("Background process")
	fo.close()

if __name__ == "__main__":
    # execute only if run as a script
    Main()