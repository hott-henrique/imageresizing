rm *.ppm

python3 ppm_generator.py

echo "File | Result" > result_testing.txt # File header

for file in '.'/*
do
	if [[ "$file" == *.ppm ]]
	then
		echo "Testing: $file"

		echo "Running matrix..."
		./tp2 $file M -w 1 -o mresult.out

		echo "Running graph..."
		./tp2 $file G -w 1 -o gresult.out

		diff_results=$(diff mresult.out gresult.out)
		len_diff=${#diff_results}

		# Save invalid results and print results in file.
		if [[ $len_diff == 0 ]]
		then
			echo -e "$file\tOK" >> result_testing.txt
			rm -f mresult.out
			rm -f gresult.out
			rm $file
		else
			echo -e "$file\tNOK" >> result_testing.txt
			mv -f mresult.out "$file"mr.ppm
			mv -f gresult.out "$file"gr.ppm
		fi

		rm -f timing.out # rm timing info if it exists.
	fi
done

cat result_testing.txt
