2016/2
Projeto de MAC0431 - Introdução a Computação Paralela e Distribuída
---------------------------------------------------------------------
Professor: Gubi

Fernanda de Camargo Magano
Renan Fichberg
Vitor Samora da Graça

---------------------------------------------------------------------
Decisões de Projeto:

Conforme surgiram dúvidas com relação a implementação, optamos por fazer duas versões com implementações distintas.
Assim sendo, apresentamos duas versões (I) e (II) para efeitos de curiosidade e comparação de resultados. :)

Note que (I) é dependente da escolha do pixel inicial de cada thread e do sentido de varredura das threads na matriz. O sentido adotado em (I) foi 
do topo superior esquerdo ao inferior direito (esquerda para a direita, cima para baixo), como a leitura de um livro em português.
Com relação ao enunciado, ainda, na parte relativa aos estouros, no caso em que este deverá acontecer e não podemos distribuir os excedentes para os vizinhos, 
o excedente é arredondado para 0.99 ou -0.99, a depender do valor.

Com relação aos possíveis deadlocks, a estratégia utilizada foi inspirada na solução do jantar dos filósofos, de tal forma que lá basta ter um filósofo canhoto 
e outros destros que resolve. Em (I), fizemos com que a uma thread par começa tentando apanhar a trava do seu pixel, enquanto que uma impar começa tentando 
apanhar a trava vizinha (a imagem é divida em setores e as fronteiras pode ter conflitos). Empiricamente, não foi detectado nenhum deadlock nos testes.
Assim, a implementação de (I) utilizou mutex das pthreads para fazer as travas.


Já com relação a (II), a matriz é dividida em blocos de acordo com a quantidade de processadores. A parte que pode ser acessada por diferentes threads é a divisa desses blocos. Assim, essas regiões são críticas e precisam ser protegidas. Se a matriz for grande, pela forma como a matriz foi percorrida (maneira usual, da esquerda para direita, de cima para baixo), quando uma thread chegar na divisa do bloco, a outra thread já vai ter concluído seu cálculo, então não terá muita concorrência na região.

O paralelismo em (II) foi obtido utilizando OpenMP com o uso de pragmas. Nos cálculos principais a matriz foi dividida em chunks e distribuída entre as threads. No cálculo da G foi possível utilizar o schedule "dynamic", já que os cálculos eram independentes. 
Foi considerado que o arquivo ppm precisa ser normalizado e que são 256 valores possíveis (intervalo [0, 255]) para normalizar de modo a ficar entre [0, 1].
No caso de estouro do valor do R ou B (para mais de 1) em alguma das iterações em que o vizinho não possa receber para não estourar também, o valor foi setado para 1 (correspondendo a 255 na hora de escrever no arquivo ppm).

Em ambas as implementações, os pixels consideram o valor corrente dos vizinhos, isto é, se baseiam nas modificações mais recentes da iteração atual, não da anterior.

