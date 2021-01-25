# ThetaStar

## Who am I? 
I am a college student, who is currently in second year Digital Arts and Entertainment. For the course “Gameplay Programming” we have been given the assignment to start an investigation, of choice about AI behavior. Below I briefly explain the research based on scientific articles and I try to implement this in my project.

# Introduction
## I.	What is A* pathfinding?
A pathfinding is an Algorithm, introduced by Peter Hart et al.,  to find the optimal path starting from the point where you are to your goal. It gets calculated by “using a heuristic function to prioritize the nodes (vertex) to be traversed, the cost is the distance it takes to walk from a node to another node” (Firmayah, Masruroh & Fahrianto, 2016, p. 275-280). The target of AI is to find the shortest but realistisc path to it’s target.
The two important steps are discretize and search. Discretion simplifies “a continuous environment* into a graph”, while searching propagates “information along this graph to find a path from a given start vertex to a given goal vertex” (Nash & Koenig, 2019). There are a lot of different ways, but game developers prefer the A * pathfinding algorithm for its simplicity and optimality guarantees. However, recent studies have been conducted on a new type of path finding, called the Theta * path finding.

![AlphaStar](https://user-images.githubusercontent.com/44691800/105644394-9e113d00-5e95-11eb-85b9-abc380d793af.png)
 
## II.	What is Theta* pathfinding? 
Theta* pathfinding is an Algorithm, introduced by Alex Nash in 2007. Theta* is very similar to A*. The biggest difference between them, is that Theta* allows the parent to be any visible vertex, when A* can only allow the parent to be a visible neighbourvertex. Theta* doesn’t have to follow a vertex that is one of his neighbours. On the image below (figure 16.2) you can clearly see that Theta* skips a few vertices, what makes it, quite frequently, shorter in a continuous environments. 

Theta* finds paths that have nearly the same length as the shortest paths in the continuous environments without the need for postprocessing technique(Firmayah, Masruroh & Fahrianto, 2016, p. 275-280).

![Thetastar](https://user-images.githubusercontent.com/44691800/105644449-e6c8f600-5e95-11eb-8034-768a8fac7f0b.png)

## III.	Which one is better?
According to the results of the study of Firmayah, Masruroh & Fahrianto A* and Theta* has the same completeness criteria and has time complecity which is relatively same. The A* pathfinding had the advantage of optimality in fewer number of nodes searched, whereas the Theta* has the advantage of the optimality the shortest route results. It can be concluded that A * is guaranteed to be the shortest path in graphs, but theta * has the shortest path in the continuous environments. In short: Theta* builds upon A*. 

* Continuous environments is an environment in which the actions performed cannot be numbered. It is not discrete, but continuous (GeekforGeeks, 2020).

# Design/implementation
I started with the framework we used for the A* pathfinding implementation in visual studio with C++. Then I went on trying to implement Theta*. The big difference between these two, is how you calculate the cost of each connection between two nodes. I had inspiration from the “Game AI Pro 360” book, the part where they discuss Theta*. With information from the internet, articles and books (see references) I tried to finish my work.

# Result
Sadly I was not yet able to fully implement Theta* pathfinding due to amount of bugs. I tried to use 2 different frameworks but both frameworks gave different errors. This results in a half working project, that needs some more refining. Because this project is based on previous studies, we know that the results end with Theta* giving the shortest path in the continuous environments.
 
# Conclusion/Future work
Because of previous findings in certain studies I explained in the introduction, we know that Theta* gives the shortest path in the continuous environments in comparison with Alpha*. The first steps are already implemented in my project, thus giving this more possibilities in the future. 
First we discuss the errors that were given: 

In the first project the error was a pointer that had the value 0xdddddd. After trying to fix the error myself, there was no other choice then search the error going trough the internet. Here I found information about the pointer already being deleted, but I couldn’t find my pointer anywhere being already deleted. This raised some questions that stayed unanswered. 

In my second project, the problem when I was trying to check whether the parent node of the connection is visible to it’s neighbor. This function sometimes got stuck in an infinite loop. I tried looking at where the problem in the function is, but sadly couldn’t find the source of the problem. 
Despite the errors, there are opportunities for improvement in the future. By finding the solution for the error, we could use this project for further research. By implementing time and other measuring techniques we could compare both (Alpha and theta) and implement this in games. This way we could see clearly how both algorithms work. 

# References
Daniel, K., Nash, A., and Koenig, S. 2010. Theta*: Any-angle path planning on grids. Journal of Artificial Intelligence Research, 39, 533–579.

Firmansyah, E. R., Masruroh, S. U., & Fahrianto, F. (2016). Comparative Analysis of A* and Basic Theta* Algorithm in Android-Based Pathfinding Games. 2016 6th International Conference on Information and Communication Technology for The Muslim World (ICT4M). https://doi.org/10.1109/ict4m.2016.063

Harabor, D. D., Grastien, A., Öz, D., & Aksakalli, V. (2016). Optimal Any-Angle Pathfinding
In Practice. Journal of Artificial Intelligence Research, 56, 89–118.
https://doi.org/10.1613/jair.5007

Nash, A., Daniel, K., Koenig, S., and Felner, A. 2007. Theta*: Any-angle path planning on grids. Proceedings of the AAAI Conference on Artificial Intelligence, 1177–1183

Rabin, S. (2020). Game AI Pro 360 (1st Edition). CRC Press.

