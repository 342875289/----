import math




p1=[-66.24,3112.90,3619.62]
p2=[-66.30,3112.85,5168.45]
p3=[1434.18,503.48,5168.39]
p4=[1434.13,503.52,3619.58]






p11=[-66.33,3112.96,3619.66]
p22=[-66.36,3112.93,5168.50]
p33=[1434.26,503.56,5168.45]
p44=[1434.23,503.54,3619.62]


print(math.sqrt( (p1[0]-p11[0])*(p1[0]-p11[0]) + (p1[1]-p11[1])*(p1[1]-p11[1]) + (p1[2]-p11[2])*(p1[2]-p11[2])  )   )
print(math.sqrt( (p2[0]-p22[0])*(p2[0]-p22[0]) + (p2[1]-p22[1])*(p2[1]-p22[1]) + (p2[2]-p22[2])*(p2[2]-p22[2])  )   )
print(math.sqrt( (p3[0]-p33[0])*(p3[0]-p33[0]) + (p3[1]-p33[1])*(p3[1]-p33[1]) + (p3[2]-p33[2])*(p3[2]-p33[2])  )   )
print(math.sqrt( (p4[0]-p44[0])*(p4[0]-p44[0]) + (p4[1]-p44[1])*(p4[1]-p44[1]) + (p4[2]-p44[2])*(p4[2]-p44[2])  )   )