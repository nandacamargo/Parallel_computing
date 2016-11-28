2016/2
Projeto de MAC0431 - Introdução a Computação Paralela e Distribuída

Professor: Gubi

Fernanda de Camargo Magano
Renan Fichberg
Vitor Samora da Graça

Decisões de Projeto:

Conforme surgiram dúvidas com relação a implementação, optamos por fazer duas versões com implementações distintas.
A dúvida, em particular, era com relação a computação das interações, de tal modo que não sabíamos se devíamos computar
considerando o valor do vizinho já modificado (I), ou se deveríamos calcular todos os pixels considerando o estado anterior e desconsiderando
a computação dos pixels vizinhos (II). Assim sendo, apresentamos duas versões distintas para efeitos de curiosidade e comparação de resultados. :)

Note que (I) é dependente da escolha do pixel inicial de cada thread e do sentido de varredura das threads na matriz. O sentido adotado em (I) foi 
do topo superior esquerdo ao inferior direito (esquerda para a direita, cima para baixo), como a leitura de um livro em português.
Com relação ao enunciado, ainda, na parte relativa aos estouros, no caso em que este deverá acontecer e não podemos distribuir os excedentes para os vizinhos, 
o excedente é arredondado para 0.99 ou -0.99, a depender do valor.

Com relação aos possíveis deadlocks, a estratégia utilizada foi inspirada na solução do jantar dos filósofos, de tal forma que lá basta ter um filosofo canhoto 
e outros destros que resolve. Em (I), fizemos com que a uma thread par começa tentando apanhar a trava do seu pixel, enquanto que uma impar começa tentando 
apanhar a trava vizinha (a imagem é divida em setores e as fronteiras pode ter conflitos). Empiricamente, não foi detectado nenhum deadlock nos testes.

Já com relação a (II), <continuar...>


