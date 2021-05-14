if [ ! -f "build/scdptopk" ]; then
	./build.sh
fi

set -e

run() {
	k=$1
	mkdir "$k"

	nr_trials=0
	until [ ! $nr_trials -lt $2 ]
	do 

		cd server 
		./scdptopk -r 0 -k $k &
		cd ../client 
		sleep 0.1
		time ./scdptopk -r 1 -k $k

		cd ..
		cat server/Selection.out > "$k/$nr_trials.out"
		cat client/Selection.out >> "$k/$nr_trials.out"

		nr_trials=`expr $nr_trials + 1 `
	done
	
	# cd ..
}

rm exp -rf && mkdir exp && cd exp
mkdir server && mkdir client
cp "../datasets/server.txt" "server/dataset.txt"
cp "../datasets/client.txt" "client/dataset.txt"
cp "../build/scdptopk" "server/"
cp "../build/scdptopk" "client/"
k=2

until [ ! $k -lt 11 ]
do
	echo k=$k
	run $k 2
	k=`expr $k + 1`
done

# rm exp -rf
# mkdir exp
# cd exp
# mkdir server
# mkdir client
# cd ..

# cp datasets/server.txt exp/server/dataset.txt
# cp datasets/client.txt exp/client/dataset.txt
# cp scdptopk exp/server/
# cp scdptopk exp/client/

# cd exp

# cd server 
# ./scdptopk -r 0 -k 4 &
# cd ..
# sleep 0.2
# cd client 
# time ./scdptopk -r 1 -k 4

# cd ..
# cat server/Selection.out > output
# cat client/Selection.out >> output


