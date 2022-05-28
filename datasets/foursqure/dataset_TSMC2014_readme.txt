This dataset includes long-term (about 10 months) check-in data in New York city and Tokyo collected from Foursquare from 12 April 2012 to 16 February 2013.
It contains two files in tsv format. Each file contains 8 columns, which are:

1. User ID (anonymized)
2. Venue ID (Foursquare)
3. Venue category ID (Foursquare)
4. Venue category name (Fousquare)
5. Latitude
6. Longitude
7. Timezone offset in minutes (The offset in minutes between when this check-in occurred and the same time in UTC)
8. UTC time

example:
470(1)	
49bbd6c0f964a520f4531fe3(2)	
4bf58dd8d48988d127951735(3)	
Arts & Crafts Store(4)	
40.719810375488535(5)	
-74.00258103213994(6)	
-240(7)	
Tue Apr 03 18:00:09 +0000 2012(8)

The file dataset_TSMC2014_NYC.txt contains 227428 check-ins in New York city.
The file dataset_TSMC2014_TKY.txt contains 573703 check-ins in Tokyo.

=============================================================================================================================
Please cite our paper if you publish material based on this dataset.

=============================================================================================================================
REFERENCES

@article{yang2014modeling,
	author={Yang, Dingqi and Zhang, Daqing and Zheng, Vincent. W. and Yu, Zhiyong},
	journal={IEEE Transactions on Systems, Man, and Cybernetics: Systems},
	title={Modeling User Activity Preference by Leveraging User Spatial Temporal Characteristics in LBSNs},
	year={2015},
	volume={45},
	number={1},
	pages={129--142},
	ISSN={2168-2216},
	publisher={IEEE}
}

