static unsigned int MA_femur_lut[256] = {
864,1052,1208,1334,1434,1509,1561,1595,1611,1611,1598,1573,1539,1496,1446,1391,1332,1270,1206,1141,1077,1014,952,894,839,788,742,700,665,635,611,594,584,580,584,594,613,638,671,711,758,812,874,942,1017,1099,1187,1282,1382,1488,1600,1716,1838,1965,2096,2231,2370,2512,2658,2807,2958,3112,3268,3426,3585,3745,3906,4068,4231,4393,4555,4717,4878,5038,5197,5354,5510,5664,5817,5967,6115,6260,6402,6542,6679,6813,6944,7071,7195,7316,7433,7547,7657,7764,7866,7966,8061,8153,8241,8326,8407,8484,8558,8629,8696,8760,8820,8878,8932,8983,9032,9078,9121,9162,9201,9237,9271,9303,9334,9362,9390,9415,9440,9464,9486,9508,9529,9550,9570,9590,9610,9630,9651,9672,9693,9715,9737,9761,9786,9811,9838,9866,9896,9927,9960,9994,10031,10069,10109,10151,10195,10241,10289,10339,10392,10446,10503,10562,10624,10687,10753,10820,10890,10962,11036,11113,11191,11271,11353,11437,11523,11610,11700,11791,11883,11977,12073,12170,12269,12369,12470,12573,12677,12782,12889,12997,13106,13216,13328,13442,13557,13674,13792,13912,14035,14159,14287,14416,14549,14686,14825,14969,15117,15271,15429,15594,15765,15943,16129,16323,16527,16741,16967,17204,17455,17720,18001,18298,18614,18949,19305,19684,20088,20518,20976,21464,21985,22540,23132,23763,24437,25155,25920,26736,27606,28532,29519,30570,31688,32878,34144,35489,36918,38437,40049,41759,43573,45497,47535,49694,51980,54398,56956,59660,62517,65535
};
//Lookup table has 256 samples from 0 to 90 degrees of femur rotation
static float femur_MA_units = 148.6707;
//Mechanical advantage is femur_MA_units rad/m per 2^16