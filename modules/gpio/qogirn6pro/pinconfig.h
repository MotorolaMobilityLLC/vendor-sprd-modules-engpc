#ifndef _PINCONFIG_H__
#define _PINCONFIG_H__
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct GPIO_ADDRESS{
 int gpio;
 int bias_address;
}GPIONODE;

/* need supplement pin table */
	GPIONODE gpionumber[]={

	{	0	,0xFFFF	},
	{	1	,0xFFFF	},
	{	2	,0xFFFF	},
	{	3	,0xFFFF	},
	{	4	,0xFFFF	},
	{	5	,0xFFFF	},
	{	6	,0xFFFF	},
	{	7	,0xFFFF	},
	{	8	,0xFFFF	},
	{	9	,0xFFFF	},
	{	10	,0xFFFF	},
	{	11	,0xFFFF	},
	{	12	,0xFFFF	},
	{	13	,0xFFFF	},
	{	14	,0xFFFF	},
	{	15	,0xFFFF	},
	{	16	,0xFFFF	},
	{	17	,0xFFFF	},
	{	18	,0xFFFF	},
	{	19	,0xFFFF	},
	{	20	,0xFFFF	},
	{	21	,0xFFFF	},
	{	22	,0xFFFF	},
	{	23	,0xFFFF	},
	{	24	,0xFFFF	},
	{	25	,0xFFFF	},
	{	26	,0xFFFF	},
	{	27	,0xFFFF	},
	{	28	,0xFFFF	},
	{	29	,0xFFFF	},
	{	30	,0xFFFF	},
	{	31	,0xFFFF	},
	{	32	,0xFFFF	},
	{	33	,0xFFFF	},
	{	34	,0xFFFF	},
	{	35	,0xFFFF	},
	{	36	,0xFFFF	},
	{	37	,0xFFFF	},
	{	38	,0xFFFF	},
	{	39	,0xFFFF	},
	{	40	,0xFFFF	},
	{	41	,0xFFFF	},
	{	42	,0xFFFF	},
	{	43	,0xFFFF	},
	{	44	,0xFFFF	},
	{	45	,0xFFFF	},
	{	46	,0xFFFF	},
	{	47	,0xFFFF	},
	{	48	,0xFFFF	},
	{	49	,0xFFFF	},
	{	50	,0xFFFF	},
	{	51	,0xFFFF	},
	{	52	,0xFFFF	},
	{	53	,0xFFFF	},
	{	54	,0xFFFF	},
	{	55	,0xFFFF	},
	{	56	,0xFFFF	},
	{	57	,0xFFFF	},
	{	58	,0xFFFF	},
	{	59	,0xFFFF	},
	{	60	,0xFFFF	},
	{	61	,0xFFFF	},
	{	62	,0xFFFF	},
	{	63	,0xFFFF	},
	{	64	,0xFFFF	},
	{	65	,0xFFFF	},
	{	66	,0xFFFF	},
	{	67	,0xFFFF	},
	{	68	,0xFFFF	},
	{	69	,0xFFFF	},
	{	70	,0xFFFF	},
	{	71	,0xFFFF	},
	{	72	,0xFFFF	},
	{	73	,0xFFFF	},
	{	74	,0xFFFF	},
	{	75	,0xFFFF	},
	{	76	,0xFFFF	},
	{	77	,0xFFFF	},
	{	78	,0xFFFF	},
	{	79	,0xFFFF	},
	{	80	,0xFFFF	},
	{	81	,0xFFFF	},
	{	82	,0xFFFF	},
	{	83	,0xFFFF	},
	{	84	,0xFFFF	},
	{	85	,0xFFFF	},
	{	86	,0xFFFF	},
	{	87	,0xFFFF	},
	{	88	,0xFFFF	},
	{	89	,0xFFFF	},
	{	90	,0xFFFF	},
	{	91	,0xFFFF	},
	{	92	,0xFFFF	},
	{	93	,0xFFFF	},
	{	94	,0xFFFF	},
	{	95	,0xFFFF	},
	{	96	,0xFFFF	},
	{	97	,0xFFFF	},
	{	98	,0xFFFF	},
	{	99	,0xFFFF	},
	{	100	,0xFFFF	},
	{	101	,0xFFFF	},
	{	102	,0xFFFF	},
	{	103	,0xFFFF	},
	{	104	,0xFFFF	},
	{	105	,0xFFFF	},
	{	106	,0xFFFF	},
	{	107	,0xFFFF	},
	{	108	,0xFFFF	},
	{	109	,0xFFFF	},
	{	110	,0xFFFF	},
	{	111	,0xFFFF	},
	{	112	,0xFFFF	},
	{	113	,0xFFFF	},
	{	114	,0xFFFF	},
	{	115	,0xFFFF	},
	{	116	,0xFFFF	},
	{	117	,0xFFFF	},
	{	118	,0xFFFF	},
	{	119	,0xFFFF	},
	{	120	,0xFFFF	},
	{	121	,0xFFFF	},
	{	122	,0xFFFF	},
	{	123	,0xFFFF	},
	{	124	,0xFFFF	},
	{	125	,0xFFFF	},
	{	126	,0xFFFF	},
	{	127	,0xFFFF	},
	{	128	,0xFFFF	},
	{	129	,0xFFFF	},
	{	130	,0xFFFF	},
	{	131	,0xFFFF	},
	{	132	,0xFFFF	},
	{	133	,0xFFFF	},
	{	134	,0xFFFF	},
	{	135	,0xFFFF	},
	{	136	,0xFFFF	},
	{	137	,0xFFFF	},
	{	138	,0xFFFF	},
	{	139	,0xFFFF	},
	{	140	,0xFFFF	},
	{	141	,0xFFFF	},
	{	142	,0xFFFF	},
	{	143	,0xFFFF	},
	{	144	,0xFFFF	},
	{	145	,0xFFFF	},
	{	146	,0xFFFF	},
	{	147	,0xFFFF	},
	{	148	,0xFFFF	},
	{	149	,0xFFFF	},
	{	150	,0xFFFF	},
	{	151	,0xFFFF	},
	{	152	,0xFFFF	},
	{	153	,0xFFFF	},
	{	154	,0xFFFF	},
	{	155	,0xFFFF	},
	{	156	,0xFFFF	},
	{	157	,0xFFFF	},
	{	158	,0xFFFF	},
	{	159	,0xFFFF	},
	{	160	,0xFFFF	},
	{	161	,0xFFFF	},
	{	162	,0xFFFF	},
	{	163	,0xFFFF	},
	{	164	,0xFFFF	},
	{	165	,0xFFFF	},
	{	166	,0xFFFF	},
	{	167	,0xFFFF	},
	{	168	,0xFFFF	},
	{	169	,0xFFFF	},
	{	170	,0xFFFF	},
	{	171	,0xFFFF	},
	{	172	,0xFFFF	},
	{	173	,0xFFFF	},
	{	174	,0xFFFF	},
	{	175	,0xFFFF	},
	{	176	,0xFFFF	},
	{	177	,0xFFFF	},
	{	178	,0xFFFF	},
	{	179	,0xFFFF	},
	{	180	,0xFFFF	},
	{	181	,0xFFFF	},
	{	182	,0xFFFF	},
	{	183	,0xFFFF	},
	{	184	,0xFFFF	},
	{	185	,0xFFFF	},
	{	186	,0xFFFF	},
	{	187	,0xFFFF	},
	{	188	,0xFFFF	},
	{	189	,0xFFFF	},
	{	190	,0xFFFF	},
	{	191	,0xFFFF	},
	{	192	,0xFFFF	},
	{	193	,0xFFFF	},
	{	194	,0xFFFF	},
	{	195	,0xFFFF	},
	{	196	,0xFFFF	},
	{	197	,0xFFFF	},
	{	198	,0xFFFF	}
	};


#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------

#endif //
