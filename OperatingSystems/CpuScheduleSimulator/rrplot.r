dvalues <- c(0, 5, 10, 15, 20, 25)

wait.q50 <- c(243325.7, 282767.27, 322035.73, 361160.38, 400174.25, 439098.94)
wait.q100 <- c(238947.11, 259081.1, 279164.7, 299216.33, 319225.72, 339200.52)
wait.q250 <- c(226401.11, 234931.39, 243453.24, 251967.71, 260473.41, 268971.61)
wait.q500 <- c(205341.3, 209937.43, 214534.67, 219124.61, 223717.21, 228304.66)

turn.q50 <- c(243852.71, 283294.28, 322562.74, 361687.39, 400701.26, 439625.95)
turn.q100 <- c(239474.13, 259608.11, 279691.71, 299743.34, 319752.73, 339727.53)
turn.q250 <- c(226928.12, 235458.4, 243980.25, 252494.72, 261000.42, 269498.62)
turn.q500 <- c(205868.31, 210464.44, 215061.68, 219651.63, 224244.22, 228831.67)

plot(dvalues, wait.q50, type="b",col="red", main="Round Robin Simulation --# of tasks 1000 --# of seeds 20", ylab="waiting time", xlab="dispatch cost", ylim=c(200000, 500000))
lines(dvalues, wait.q100, col="blue", type="b")
lines(dvalues, wait.q250, col="orange", type="b")
lines(dvalues, wait.q500, col="pink", type="b")
legend(0, 500000, c("q50", "q100", "q250", "q500"), col=c("red", "blue", "orange", "pink"), title="Quantum Values", lty=1)

plot(dvalues, turn.q50, type="b",col="red", main="Round Robin Simulation --# of tasks 1000 --# of seeds 20", ylab="turnaround time", xlab="dispatch cost", ylim=c(200000, 500000))
lines(dvalues, turn.q100, col="blue", type="b")
lines(dvalues, turn.q250, col="orange", type="b")
lines(dvalues, turn.q500, col="pink", type="b")
legend(0, 500000, legend=c("q50", "q100", "q250", "q500"), col=c("red", "blue", "orange", "pink"), title="Quantum Values", lty=1)
