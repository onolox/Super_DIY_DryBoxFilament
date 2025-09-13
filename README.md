## Este é um projeto de uma caixa para secagem de filamentos DIY super poderosa caseira. 

As caixas vendidas no mercado tem um sério problema além de serem muito caras, não tem isolamento térmico, com isso há um desperdício enorme de energia.
Além disso não é necessário um fluxo grande de ar saindo e entrando da caixa, só um pequeno furo de entrada e saída. O que é bom ter é um fluxo grande de ar circulando internamente.
Com isso em mente resolvi fazer a minha própria caixa com coisas que eu tinha em casa.


### Lista de itens para a caixa:
- Caixa de papelão ( o tamanho depende do que vote quiser, no meu caso couberam 4 rolos);
- 2 placas de isopor de 2~3cm (Forração interna e a tampa);
- Papel aluminio;
- Cola de madeira;
- 4 coolers;
- Soquete para a lampada de cerâmica
- Lampada de aquecimento de 100W
- Fio de 1.5mm
- Dissipador velho de CPU
- Pasta térmica


### Lista:
- RP2040
- Sensor DHT22
- Fonte 12v, 1A
- Step-Down para 5V
- Tela LCD 2.4 ILI9341
- Dimmer DIY ou comprado
- Buzzer

A caixa deve ser forrada com isopor e papel aluminio, os cantos arredondados usei partes quadradas cortadas curvas com fio quente de uma fonte de bancada. 
Na tampa usei os pedaços que sobraram para fazer o contorno interno. 
Um furo embaixo onde o ar é sugado e um em cima.
A lampada deve ser "colada" com bastante pasta térmica ao dissipador. A lampada pode ser usada sem nada, porém vai dificultar bastante a dissipação de calor. Ela sozinha funciona a 450°C, já com o dissipador a 150ºC, gerando uma vida útil enorme.
A caixa pode claro ser melhorada colocando mais lampadas para acelerar o aquecimento incicial.
Para o Dimmer usei esse projeto: [Arduino Dimmer](https://www.instructables.com/Arduino-controlled-light-dimmer-The-circuit/ "Arduino Dimmer"), mas pode ser comprado da china, porém acho muito caro, pelo preço da loja da pra comprar 10x em componentes.

A parte realmente difícil que me tomou muitos meses foi em relação ao software, aqui é onde está o ouro. Usei o PlatformI0 no vSCode.



### Fotos do projeto:
|||
| ------------ | ------------ |
|<img src="/Images_Files/1.jpg" width="400px" /> |<img src="/Images_Files/2.jpg" width="400px" /> |
|<img src="/Images_Files/3.jpg" width="400px" /> |<img src="/Images_Files/4.jpg" width="400px" /> |
|<img src="/Images_Files/5.jpg" width="400px" /> |<img src="/Images_Files/6.jpg" width="400px" /> |
|<img src="/Images_Files/7.jpg" width="400px" /> |<img src="/Images_Files/8.jpg" width="400px" /> |
|<img src="/Images_Files/screen running.jpg" width="400px" /> |<img src="/Images_Files/rp2040 YD black.webp" width="400px" /> |




### Dica para fixar os parafusos: Faça o furo e rosqueie os parafusos, depois coloque super cola nos buracos, isso reforçará os furos:
<img src="/Images_Files/Mounting holes.jpg" width="400px" />


### Foto da caixa com a eletronica:

<img src="/Images_Files/Board 1.jpg" width="400px" />


[Projeto Fusion 360 da caixa onde fica a eletrônica para usar de base.](/Images_Files/DIY Filament Dry Box.f3d "Projeto Fusion 360 da caixa onde fica a eletrônica para usar de base.")

# Aqui se encontra toda a base para você criar a sua caixa para secar filamentos, melhore, compartilhe.
