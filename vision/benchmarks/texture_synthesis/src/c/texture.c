/********************************
Author: Sravanthi Kota Venkata
********************************/

#include "texture.h"
#include <math.h>

//int *xloopin, *yloopin;

/*********************************
This is the main texture synthesis function. Called just once
from main to generate image from 'image' into 'result'.
Synthesis parameters (image and neighborhood sizes) are in global
'data-> structure.
*********************************/

void create_texture(F2D *image, params *data,F2D* result,F2D* target,int* atlas,int* xloopout,int* yloopout)
{
int anotherpass=0,maxcand = 40;
int* candlistx=NULL, *candlisty=NULL;
int vrstartx=0, vrfinishx=0, vrstarty=0, vrfinishy=0;
    int i=0,j=0,k=0, ncand=0, bestx=0,besty=0;
    double diff=0,curdiff=0;
    int tsx=0,tsy=0;

    candlistx = (int*)malloc(sizeof(int)*(data->localx*(data->localy+1)+1));
    candlisty = (int*)malloc(sizeof(int)*(data->localx*(data->localy+1)+1));

//    printf("total = %d\t%d\t%d\n", data->localx, data->localy, (data->localx*(data->localy+1)+1));

    if(!anotherpass) init(image,data,atlas,target,result, &vrstartx, &vrstarty,&vrfinishx,&vrfinishy,anotherpass);

    for(i=0;i<data->heightout-data->localy/2;i++)
    {
        for(j=0;j<data->widthout;j++)
        {
            // First, create a list of candidates for particular pixel.
		if(anotherpass) ncand = create_all_candidates(j,i, data,atlas,xloopout,yloopout,vrstartx,vrstarty,vrfinishx,vrfinishy,candlistx,candlisty);
		else  ncand = create_candidates(j,i, data,atlas,xloopout,yloopout,vrstartx,vrstarty,vrfinishx,vrfinishy,candlistx,candlisty);

            // If there are multiple candidates, choose the best based on L_2 norm

            if(ncand > 1)
            {
                diff = 1e10;
                for(k=0;k<ncand;k++)
                {
                    curdiff = compare_neighb(image, arrayref(candlistx,k),arrayref(candlisty,k),result,j,i, data,xloopout,yloopout);
                    curdiff += compare_rest(image,arrayref(candlistx,k),arrayref(candlisty,k),target,j,i,data,xloopout,yloopout);
                    if(curdiff < diff)
                    {
                        diff = curdiff;
                        bestx = arrayref(candlistx,k);
                        besty = arrayref(candlisty,k);
                    }
                }
            }
            else
            {
                bestx = arrayref(candlistx,0);
                besty = arrayref(candlisty,0);
            }

            // Copy the best candidate to the output image and record its position
            // in the atlas (atlas is used to create candidates)

            asubsref(result,a(j,i,data->widthout)+R) = asubsref(image,a(bestx,besty,data->widthin)+R);
//            asubsref(result,a(j,i,data->widthout)+G) = asubsref(image,a(bestx,besty,data->widthin)+G);
//            asubsref(result,a(j,i,data->widthout)+B) = asubsref(image,a(bestx,besty,data->widthin)+B);
            arrayref(atlas,aa(j,i)) = bestx;
            arrayref(atlas,aa(j,i)+1) = besty;
        }
    }

    // Use full neighborhoods for the last few rows. This is a small
    // fraction of total area - can be ignored for optimization purposes.

    for(;i<data->heightout;i++)
    {
        for(j=0;j<data->widthout;j++)
        {
		ncand = create_all_candidates(j,i,data,atlas,xloopout,yloopout,vrstartx,vrstarty,vrfinishx,vrfinishy,candlistx,candlisty);
            if(ncand > 1)
            {
                diff = 1e10;
                for(k=0;k<ncand;k++)
                {
                    curdiff = compare_full_neighb(image, arrayref(candlistx,k),arrayref(candlisty,k),result,j,i, data,xloopout,yloopout);
                    if(curdiff < diff)
                    {
                        diff = curdiff;
                        bestx = arrayref(candlistx,k);
                        besty = arrayref(candlisty,k);
                    }
                }
            }
            else
            {
                bestx = arrayref(candlistx,0);
                besty = arrayref(candlisty,0);
            }

            asubsref(result,a(j,i,data->widthout)+R) = asubsref(image,a(bestx,besty,data->widthin)+R);
//            asubsref(result,a(j,i,data->widthout)+G) = asubsref(image,a(bestx,besty,data->widthin)+G);
//            asubsref(result,a(j,i,data->widthout)+B) = asubsref(image,a(bestx,besty,data->widthin)+B);
            arrayref(atlas,aa(j,i)) = bestx;
            arrayref(atlas,aa(j,i)+1) = besty;
        }
    }

    /*********************************
    End of main texture synthesis loop
    *********************************/

    for(i=0;i<data->localy/2;i++)
    {
        for(j=0;j<data->widthout;j++)
        {
		ncand = create_all_candidates(j,i,data,atlas,xloopout,yloopout,vrstartx,vrstarty,vrfinishx,vrfinishy,candlistx,candlisty);
            if(ncand > 1)
            {
                diff = 1e10;
                for(k=0;k<ncand;k++)
                {
                    curdiff = compare_full_neighb(image,arrayref(candlistx,k),arrayref(candlisty,k),result,j,i, data,xloopout,yloopout);
                    if(curdiff < diff)
                    {
                        diff = curdiff;
                        bestx = arrayref(candlistx,k);
                        besty = arrayref(candlisty,k);
                    }
                }
            }
            else
            {
                bestx = arrayref(candlistx,0);
                besty = arrayref(candlisty,0);
            }

            asubsref(result,a(j,i,data->widthout)+R) = asubsref(image,a(bestx,besty,data->widthin)+R);
//            asubsref(result,a(j,i,data->widthout)+G) = asubsref(image,a(bestx,besty,data->widthin)+G);
//            asubsref(result,a(j,i,data->widthout)+B) = asubsref(image,a(bestx,besty,data->widthin)+B);
            arrayref(atlas,aa(j,i)) = bestx;
            arrayref(atlas,aa(j,i)+1) = besty;
        }
    }
    free(candlistx);
    free(candlisty);
}

// Creates a list of valid candidates for given pixel using only L-shaped causal area

int create_candidates(int x,int y, params* data,int* atlas,int* xloopout,int* yloopout, int vrstartx,int vrstarty,int vrfinishx,int vrfinishy,int* candlistx, int* candlisty)
{
    int address=0,i=0,j=0,k=0,n = 0;
    for(i=0;i<=data->localy/2;i++)
    {
        for(j=-data->localx/2;j<=data->localx/2;j++)
        {
            if(i==0 && j>=0)
                continue;
            address = aa( arrayref(xloopout,x+j), arrayref(yloopout,y-i) );
            arrayref(candlistx,n) = arrayref(atlas,address) - j;
            arrayref(candlisty,n) = arrayref(atlas,address+1) + i;

            if( arrayref(candlistx,n) >= vrfinishx || arrayref(candlistx,n) < vrstartx)
            {
                arrayref(candlistx,n) = vrstartx + (int)(drand48()*(vrfinishx-vrstartx));
                arrayref(candlisty,n) = vrstarty + (int)(drand48()*(vrfinishy-vrstarty));
                n++;
                continue;
            }

            if( arrayref(candlisty,n) >= vrfinishy )
            {
                arrayref(candlisty,n) = vrstarty + (int)(drand48()*(vrfinishy-vrstarty));
                arrayref(candlistx,n) = vrstartx + (int)(drand48()*(vrfinishx-vrstartx));
                n++;
                continue;
            }

            for(k=0;k<n;k++)
            {
                if( arrayref(candlistx,n) == arrayref(candlistx,k) && arrayref(candlisty,n) == arrayref(candlisty,k))
                {
                    n--;
                    break;
                }
            }
            n++;
        }
    }

    return n;
}

// Created a list of candidates using the complete square around the pixel

int create_all_candidates(int x,int y, params* data,int* atlas,int* xloopout,int* yloopout, int vrstartx,int vrstarty,int vrfinishx,int vrfinishy,int* candlistx, int* candlisty)
{
    int address=0,i=0,j=0,k=0,n = 0;
    for(i=-data->localy/2;i<=data->localy/2;i++)
    {
        for(j=-data->localx/2;j<=data->localx/2;j++)
        {
            if(i==0 && j>=0)
                continue;
//            printf("Entering = (%d,%d)\n", i,j);
            address = aa( arrayref(xloopout,x+j), arrayref(yloopout,y-i) );
            arrayref(candlistx,n) = arrayref(atlas,address)-j;
            arrayref(candlisty,n) = arrayref(atlas,address+1)+i;

            if( arrayref(candlistx,n) >= vrfinishx || arrayref(candlistx,n) < vrstartx)
            {
                arrayref(candlistx,n) = vrstartx + (int)(drand48()*(vrfinishx-vrstartx));
                arrayref(candlisty,n) = vrstarty + (int)(drand48()*(vrfinishy-vrstarty));
                n++;
//                printf("1: (%d,%d)\t%d\n", i,j,n);
                continue;
            }

            if( arrayref(candlisty,n) >= vrfinishy || arrayref(candlisty,n) < vrstarty)
            {
                arrayref(candlisty,n) = vrstarty + (int)(drand48()*(vrfinishy-vrstarty));
                arrayref(candlistx,n) = vrstartx + (int)(drand48()*(vrfinishx-vrstartx));
                n++;
//                printf("2: (%d,%d)\t%d\n", i,j,n);
                continue;
            }

            for(k=0;k<n;k++)
            {
                if( arrayref(candlistx,n) == arrayref(candlistx,k) && arrayref(candlisty,n) == arrayref(candlisty,k) )
                {
                    n--;
//                    printf("3: (%d,%d)\t%d\n", i,j,n);
                    break;
                }
            }
            n++;
//            printf("4: (%d,%d)\t%d\n", i,j,n);
        }
    }

    return n;
}

// Initializes the output image and atlases to a random collection of pixels

void init(F2D *image, params* data,int* atlas,F2D* target,F2D* result, int* vrstartx,int* vrstarty,int* vrfinishx,int* vrfinishy,int anotherpass)
{
    int i=0,j=0,tmpx=0,tmpy=0;
    *vrstartx = data->localx/2; *vrstarty = data->localy/2;
    *vrfinishx = data->widthin-data->localx/2;
    *vrfinishy = data->heightin-data->localy/2;
    for(i=0;i<data->heightout;i++)
    {
        for(j=0;j<data->widthout;j++)
        {
            if(
            asubsref(target,a(j,i,data->widthout)+R) == 1.0
//            && asubsref(target,a(j,i,data->widthout)+G) == 1.0
//            && asubsref(target,a(j,i,data->widthout)+B) == 1.0
            )
            {
                tmpx = *vrstartx + (int)(drand48()*((*vrfinishx)-(*vrstartx)));
                tmpy = *vrstarty + (int)(drand48()*((*vrfinishy)-(*vrstarty)));
                if(!anotherpass)
                {
                    arrayref(atlas,aa(j,i)) = tmpx;
                    arrayref(atlas,aa(j,i)+1) = tmpy;
                    asubsref(result,a(j,i,data->widthout)+R) = asubsref(image,a(tmpx,tmpy,data->widthin)+R);
//                    asubsref(result,a(j,i,data->widthout)+G) = asubsref(image,a(tmpx,tmpy,data->widthin)+G);
//                    asubsref(result,a(j,i,data->widthout)+B) = asubsref(image,a(tmpx,tmpy,data->widthin)+B);
                }
            }
        }
    }

    return;
}


// Compares two square neighborhoods, returns L_2 difference

double compare_full_neighb(F2D *image,int x, int y, F2D *image1,int x1, int y1, params* data,int* xloopout,int* yloopout)
{
    double tmp=0,res = 0;
    int i=0,j=0,addr=0,addr1=0;
    for(i=-(data->localy/2);i<=data->localy/2;i++)
    {
        for(j=-(data->localx/2);j<=data->localx/2;j++)
        {
            if( !( i > 0 && y1 > data->localy && y1+i < data->heightout) )
            {
                addr = a(x+j,y+i,data->widthin);
                addr1 = a( arrayref(xloopout,x1+j), arrayref(yloopout,y1+i), data->widthout);

                tmp = asubsref(image,addr+R) - asubsref(image1,addr1+R);
                res += tmp*tmp;
//                tmp = asubsref(image,addr+G) - asubsref(image1,addr1+G);
//                res += tmp*tmp;
//                tmp = asubsref(image,addr+B) - asubsref(image1,addr1+B);
//                res += tmp*tmp;
            }
        }
    }

    return res;
}

// Compares two L-shaped neighborhoods, returns L_2 difference

double compare_neighb(F2D *image,int x, int y, F2D *image1,int x1, int y1, params* data,int* xloopout,int* yloopout)
{
    double tmp=0,res = 0;
    int i=0,j=0,addr1=0,addr=0;
    for(i=-(data->localy/2);i<0;i++)
    {
        for(j=-(data->localx/2);j<=data->localx/2;j++)
        {
            addr = a(x+j,y+i,data->widthin);
            addr1 = a( arrayref(xloopout,x1+j), arrayref(yloopout,y1+i), data->widthout);

            tmp = asubsref(image,addr+R) - asubsref(image1,addr1+R);
            res += tmp*tmp;
//            tmp = asubsref(image,addr+G) - asubsref(image1,addr1+G);
//            res += tmp*tmp;
//            tmp = asubsref(image,addr+B) - asubsref(image1,addr1+B);
//            res += tmp*tmp;
        }
    }

    for(j=-(data->localx/2);j<0;j++)
    {
        addr = a(x+j,y,data->widthin);
        addr1 = a( arrayref(xloopout,x1+j), y1, data->widthout);

        tmp = asubsref(image,addr+R) - asubsref(image1,addr1+R);
        res += tmp*tmp;
//        tmp = asubsref(image,addr+G) - asubsref(image1,addr1+G);
//        res += tmp*tmp;
//        tmp = asubsref(image,addr+B) - asubsref(image1,addr1+B);
//        res += tmp*tmp;
    }

    return res;
}

double compare_rest(F2D *image,int x, int y, F2D *tar,int x1, int y1, params* data,int* xloopout,int* yloopout)
{
    double tmp=0,res = 0;
    int i=0,j=0,addr=0,addr1=0;

    for(i=(data->localy/2);i>0;i--)
    {
        for(j=-(data->localx/2);j<=data->localx/2;j++)
        {
            addr = a(x+j,y+i,data->widthin);
            addr1 = a( arrayref(xloopout,x1+j), arrayref(yloopout,y1+i), data->widthout);

            if( asubsref(tar,addr1+R) != 1.0) //KVS?
            {
                tmp = asubsref(image,addr+R) - asubsref(tar,addr1+R);
                res += tmp*tmp;
//                tmp = asubsref(image,addr+G) - asubsref(tar,addr1+G);
//                res += tmp*tmp;
//                tmp = asubsref(image,addr+B) - asubsref(tar,addr1+B);
//                res += tmp*tmp;
            }
        }
    }

    for(j=(data->localx/2);j>0;j--)
    {
        addr = a(x+j,y,data->widthin);
        addr1 = a( arrayref(xloopout,x1+j), y1, data->widthout);
        if( asubsref(tar,addr1+R) != 1.0)   // KVS?
        {
            tmp = asubsref(image,addr+R) - asubsref(tar,addr1+R);
            res += tmp*tmp;
//            tmp = asubsref(image,addr+G) - asubsref(tar,addr1+G);
//            res += tmp*tmp;
//            tmp = asubsref(image,addr+B) - asubsref(tar,addr1+B);
//            res += tmp*tmp;
        }
    }

    return res;
}

