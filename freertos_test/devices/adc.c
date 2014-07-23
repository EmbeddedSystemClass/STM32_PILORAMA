#include "adc.h"
#include <stm32f4xx.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_adc.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "tablo.h"
#include "tablo_parser.h"

extern struct tablo tab;//

static void ADC_Task(void *pvParameters);

void ADC_Channel_Init(void)
{
	   ADC_InitTypeDef ADC_InitStructure;
	   ADC_StructInit(&ADC_InitStructure);

	   ADC_CommonInitTypeDef adc_init;
	   ADC_CommonStructInit(&adc_init);

	   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	   GPIO_InitTypeDef gpio;
	   GPIO_StructInit(&gpio);
	   gpio.GPIO_Mode = GPIO_Mode_AN;
	   gpio.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2;
	   GPIO_Init(GPIOA, &gpio);

	   /* ��������� ������������ A��1 */
	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	   /* ���������� ��������� ��� */
	   ADC_DeInit();

	   /* ���1 � ���2 �������� ���������� */
	   adc_init.ADC_Mode = ADC_Mode_Independent;
	   adc_init.ADC_Prescaler = ADC_Prescaler_Div2;

	   /* ������������� */
	   ADC_CommonInit(&adc_init);


	   ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;
	   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	   ADC_InitStructure.ADC_NbrOfConversion=1;
	   ADC_Init(ADC1, &ADC_InitStructure);

	   ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);


	   /* �������� ���1 */
	   ADC_Cmd(ADC1, ENABLE);

	   xTaskCreate(ADC_Task,(signed char*)"ADC",128,NULL, tskIDLE_PRIORITY + 1, NULL);
}

#define SWAP(A, B) { uint16_t t = A; A = B; B = t; }

void bubblesort(uint16_t *a, uint16_t n)
{
  uint16_t i, j;

  for (i = n - 1; i > 0; i--)
  {
    for (j = 0; j < i; j++)
    {
      if (a[j] > a[j + 1])
        SWAP( a[j], a[j + 1] );
    }
  }
}

#define NUM_CONV	16
static void ADC_Task(void *pvParameters)
{
		uint32_t ADC_result=0;
		uint8_t i=0;
		uint8_t str_buf[8];
		uint16_t adc_buf[NUM_CONV];
		while(1)
		{
			  ADC_result=0;
			  for(i=0;i<NUM_CONV;i++)
			  {
				  ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART;//ADC_SoftwareStartConv(ADC1);
				   while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
				   {
					   taskYIELD ();
				   }

				   adc_buf[i]=ADC1->DR;/*ADC_GetConversionValue(ADC1)*/;
			  }
			  bubblesort(adc_buf,NUM_CONV);
			  ADC_result=((adc_buf[(NUM_CONV>>1)-1]+adc_buf[NUM_CONV>>1])>>1);
			  sprintf(str_buf,"%4u",ADC_result);
			  str_to_ind(&tab.indicators[0],str_buf);
			  vTaskDelay(400);
		}
}
