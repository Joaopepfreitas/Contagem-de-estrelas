# Contagem-de-estrelas
Contagem paralela de estrelas em imagens PGM usando o modelo mestre/escravo.

🌠 Contagem de Estrelas em Imagens PGM — Processamento Paralelo (Modelo Mestre/Escravo)
🧩 Descrição Geral

Este projeto implementa um sistema para identificar e contar estrelas em imagens astronômicas no formato PGM (Portable GrayMap), utilizando 256 tons de cinza.
O objetivo é detectar regiões de pixels com valores próximos ao branco, que representam as estrelas captadas por um telescópio em uma imagem noturna.

O processamento é realizado de forma paralela, adotando o modelo Mestre/Escravo (Master/Slave), em que o processo mestre coordena a execução e distribui partes da imagem para os processos escravos realizarem o cálculo de forma independente.

⚙️ Funcionamento

A imagem PGM é lida pelo processo mestre (processo zero).

O mestre divide a imagem em partes iguais na vertical e na horizontal, de acordo com a quantidade de processos disponíveis.

Cada processo escravo recebe uma das partes, identifica as regiões de pixels com valores próximos ao branco e conta as estrelas presentes.

Após o processamento, cada escravo devolve sua contagem ao mestre, que pode então enviar um novo pedaço da imagem até que todo o arquivo seja processado.

Por fim, o mestre soma todas as contagens e apresenta ao usuário o total de estrelas encontradas.

🧠 Arquitetura e Modelo de Execução

Mestre (processo 0):

Leitura da imagem PGM.

Divisão da imagem em sub-blocos.

Distribuição das tarefas para os escravos.

Recebimento das contagens parciais e soma dos resultados.

Exibição da contagem final ao usuário.

Escravos:

Recebem uma parte da imagem.

Processam a região identificando pixels de alta intensidade (valores próximos ao branco).

Calculam a quantidade de estrelas e devolvem o resultado ao mestre.

🚀 Objetivos do Projeto

Explorar o paralelismo em sistemas distribuídos.

Aplicar o modelo Mestre/Escravo em um problema real de processamento de imagem.

Reduzir o tempo de execução por meio da divisão de tarefas entre múltiplos processos.

Demonstrar a aplicação prática de comunicação entre processos e balanceamento de carga.

🧰 Tecnologias Utilizadas

Linguagem: C 
Modelo de Paralelismo: MPI (Message Passing Interface) / Sockets

Formato de Imagem: PGM (Portable GrayMap – 256 tons de cinza)

🎯 Aplicação

Este projeto é parte da disciplina de Computação Paralela e Distribuída do curso de Ciência da Computação da UNIFAL-MG, e tem como finalidade a aplicação prática de conceitos de processamento paralelo, distribuição de tarefas, sincronização e comunicação entre processos em um cenário de análise de imagens astronômicas.

🏫 Instituição

Universidade Federal de Alfenas (UNIFAL-MG)
Curso: Ciência da Computação — 4º Período
Disciplina: Computação Paralela e Distribuída
