### <div align="center">Sistema de Cofre com Autenticação de 4 dígitos</div>  
  

- Projeto desenvolvido em C com uso da placa [BitDogLab](https://github.com/BitDogLab/BitDogLab).  
  

- Desenvolvido para fim de estudos no projeto Embarcatech - IFRN.


- Baseado na seguinte [documentação](https://github.com/MoisesLuc/BitDogLab-C/tree/main/neopixel_pio) da BitDogLab.

<br/>  

## Funcionamento

- Todo o projeto depende apenas do uso dos dois botões da placa para funcionar. Normalmente eles operam em modo "edge-triggered" e rising edge, ou seja, a ativação deles só é lida ao pressionar e soltar o botão.

- O Botão A é responsável por incrementar o valor onde se encontra o apontador atualmente (barra abaixo do dígito), indo de 0 a 9 e resetando ao ultrapassar o limite.

- O Botão B move o apontador para a direita para que seja possível incrementar os outros dígitos, retornando ao primeiro dígito da esquerda após passar do quarto dígito.

- Pressionando o botão A e B simultaneamente, é feita a comparação e autenticação entre a senha inserida e a senha de abertura do cofre, definida na função "auth()" dentro do código.

- Após a autenticação, a matriz de LED é acionada com base no resultado, mostrando um cadeado verde ou vermelho em caso da senha estar correta ou errada.
