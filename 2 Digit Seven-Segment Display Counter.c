byte s[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};
int p[]={2,3,5,6,8,4,9}, d[]={10,11}, c=0;
unsigned long m=0;

void setup(){for(int i=0;i<7;i++)pinMode(p[i],OUTPUT);for(int i=0;i<2;i++)pinMode(d[i],OUTPUT);}

void loop(){if(millis()-m>=1000){m=millis();c=(c+1)%11;}show(c);}

void show(int n){if(n<10){digitalWrite(d[0],HIGH);digit(n,1);delay(5);for(int i=0;i<7;i++)digitalWrite(p[i],LOW);digitalWrite(d[1],HIGH);}else{digit(1,0);delay(5);for(int i=0;i<7;i++)digitalWrite(p[i],LOW);digitalWrite(d[0],HIGH);digit(0,1);delay(5);}}

void digit(int n,int i){for(int j=0;j<7;j++)digitalWrite(p[j],LOW);digitalWrite(d[0],HIGH);digitalWrite(d[1],HIGH);for(int j=0;j<7;j++)digitalWrite(p[j],s[n][j]);digitalWrite(d[i],LOW);}