#include "LKH_embeded.h"
#include "../lib/LKH/SRC/INCLUDE/LKH.h"
#include "../lib/LKH/SRC/INCLUDE/Heap.h"
// ERXT
#include "../lib/LKH/SRC/INCLUDE/Genetic.h"
#include "LKH0.h"

static const Context *ctx = NULL;
static int *path = NULL;

static void ReadParameters_embeded() {
    ProblemFileName = PiFileName = InputTourFileName =
        OutputTourFileName = TourFileName = 0;
    CandidateFiles = MergeTourFiles = 0;
    AscentCandidates = 50;
    BackboneTrials = 0;
    Backtracking = 0;
    BWTSP_B = 0;
    BWTSP_Q = 0;
    BWTSP_L = INT_MAX;
    CandidateSetSymmetric = 0;
    CandidateSetType = ALPHA;
    Crossover = ERXT;
    DelaunayPartitioning = 0;
    DelaunayPure = 0;
    DemandDimension = 1;
    DistanceLimit = DBL_MAX;
    Excess = -1;
    ExternalSalesmen = 0;
    ExtraCandidates = 0;
    ExtraCandidateSetSymmetric = 0;
    ExtraCandidateSetType = QUADRANT;
    Gain23Used = 1;
    GainCriterionUsed = 1;
    GridSize = 1000000.0;
    InitialPeriod = -1;
    InitialStepSize = 0;
    InitialTourAlgorithm = WALK;
    InitialTourFraction = 1.0;
    KarpPartitioning = 0;
    KCenterPartitioning = 0;
    KMeansPartitioning = 0;
    Kicks = 1;
    KickType = 0;
    MaxBreadth = INT_MAX;
    MaxCandidates = 5;
    MaxPopulationSize = 0;
    MaxSwaps = -1;
    MaxTrials = -1;
    MoorePartitioning = 0;
    MoveType = 5;
    MoveTypeSpecial = 0;
    MTSPDepot = 1;
    MTSPMinSize = 1;
    MTSPMaxSize = -1;
    MTSPObjective = -1;
    NonsequentialMoveType = -1;
    Optimum = MINUS_INFINITY;
    PatchingA = 1;
    PatchingC = 0;
    PatchingAExtended = 0;
    PatchingARestricted = 0;
    PatchingCExtended = 0;
    PatchingCRestricted = 0;
    Precision = 100;
    Probability = 100;
    POPMUSIC_InitialTour = 0;
    POPMUSIC_MaxNeighbors = 5;
    POPMUSIC_SampleSize = 10;
    POPMUSIC_Solutions = 50;
    POPMUSIC_Trials = 1;
    Recombination = IPT;
    RestrictedSearch = 1;
    RohePartitioning = 0;
    Runs = 0;
    Salesmen = 1;
    Scale = -1;
    Seed = 1;
    SierpinskiPartitioning = 0;
    StopAtOptimum = 1;
    Subgradient = 1;
    SubproblemBorders = 0;
    SubproblemsCompressed = 0;
    SubproblemSize = 0;
    SubsequentMoveType = 0;
    SubsequentMoveTypeSpecial = 0;
    SubsequentPatching = 1;
    TimeLimit = DBL_MAX;
    TotalTimeLimit = DBL_MAX;
    TraceLevel = 1;
    TSPTW_Makespan = 0;

    // SPECIAL
    Gain23Used = 0;
    KickType = 4;
    MaxSwaps = 0;
    MoveType = 5;
    MoveTypeSpecial = 1;
    MaxPopulationSize = 10;
    // MTSP_OBJECTIVE = MINSUM
    MTSPObjective = MINSUM;
    // TRACE_LEVEL = 0
    TraceLevel = 0;
    // MAX_CANDIDATES = 6
    MaxCandidates = 6;
    // MAX_TRIALS = 10000
    MaxTrials = 10000;
    // RUNS = 1
    Runs = 1;
    // INITIAL_TOUR_ALGORITHM = GREEDY
    InitialTourAlgorithm = GREEDY;
    // TIME_LIMIT = 20
    TimeLimit = 20.0;

}

static void CreateNodes() {
    Node *Prev = 0, *N = 0;
    int i;

    if (Dimension <= 0)
        eprintf("DIMENSION is not positive (or not specified)");
    if (Asymmetric) {
        Dim = DimensionSaved;
        DimensionSaved = Dimension + Salesmen - 1;
        Dimension = 2 * DimensionSaved;
    }
    else if (ProblemType == HPP) {
        Dimension++;
        if (Dimension > MaxMatrixDimension)
            eprintf("DIMENSION too large in HPP problem");
    }
    NodeSet = (Node *)calloc(Dimension + 1, sizeof(Node));
    for (i = 1; i <= Dimension; i++, Prev = N) {
        N = &NodeSet[i];
        if (i == 1)
            FirstNode = N;
        else
            Link(Prev, N);
        N->Id = N->OriginalId = i;
        if (MergeTourFiles >= 1)
            N->MergeSuc = (Node **)calloc(MergeTourFiles, sizeof(Node *));
        N->Earliest = 0;
        N->Latest = INT_MAX;
    }
    Link(N, FirstNode);
}

static int FixEdge(Node *Na, Node *Nb) {
    if (!Na->FixedTo1 || Na->FixedTo1 == Nb)
        Na->FixedTo1 = Nb;
    else if (!Na->FixedTo2 || Na->FixedTo2 == Nb)
        Na->FixedTo2 = Nb;
    else
        return 0;
    if (!Nb->FixedTo1 || Nb->FixedTo1 == Na)
        Nb->FixedTo1 = Na;
    else if (!Nb->FixedTo2 || Nb->FixedTo1 == Na)
        Nb->FixedTo2 = Na;
    else
        return 0;
    return 1;
}


static void Read_EDGE_WEIGHT_SECTION() {
    Node *Ni;
    int i, j, n, W;
    double w;


    n = Dimension;
    if (!FirstNode)
        CreateNodes();
    n = Dimension / 2;
    CostMatrix = (int *)calloc((size_t)n * n, sizeof(int));
    for (Ni = FirstNode; Ni->Id <= n; Ni = Ni->Suc)
        Ni->C = &CostMatrix[(size_t)(Ni->Id - 1) * n] - 1;
    if (Scale < 1)
        Scale = 1;
    switch (WeightFormat) {
        case FULL_MATRIX:
            for (i = 1; i <= Dim; i++) {
                Ni = &NodeSet[i];
                for (j = 1; j <= Dim; j++) {
                    //
                    w = getNodeDist(i - 1, j - 1, ctx);
                    W = round(Scale * w);
                    Ni->C[j] = W;
                    if (j != i && W > M)
                        M = W;
                }
            }
            break;
    }
    for (i = 1; i <= DimensionSaved; i++)
        FixEdge(&NodeSet[i], &NodeSet[i + DimensionSaved]);
    Distance = Distance_ATSP;
    WeightType = -1;
}

static void ReadProblem_embeded() {
    FreeStructures();
    FirstNode = 0;
    WeightType = WeightFormat = ProblemType = -1;
    CoordType = NO_COORDS;
    Name = NULL;
    Type = EdgeWeightType = EdgeWeightFormat = 0;
    EdgeDataFormat = NodeCoordType = DisplayDataType = 0;
    Distance = 0;
    C = 0;
    c = 0;

    // Read_TYPE();
    // TYPE : ATSP
    ProblemType = ATSP;
    Asymmetric = ProblemType == ATSP;

    // Read_DIMENSION();
    // DIMENSION : ...
    Dimension = ctx->input->ioVec.len;
    DimensionSaved = Dim = Dimension;

    // Read_SALESMEN();
    // VEHICLES : 1
    Salesmen = 1;

    // Read_EDGE_WEIGHT_TYPE();
    // EDGE_WEIGHT_TYPE: EXPLICIT
    WeightType = EXPLICIT;
    Distance = Distance_EXPLICIT;
    if (Scale < 1)
        Scale = 1;

    // Read_EDGE_WEIGHT_FORMAT();
    // EDGE_WEIGHT_FORMAT: FULL_MATRIX
    WeightFormat = FULL_MATRIX;

    Read_EDGE_WEIGHT_SECTION();

    int i, j, K;

    Swaps = 0;

    /* Adjust parameters */
    if (Seed == 0)
        Seed = (unsigned)(time(0) * (size_t)(&Seed));
    if (Precision == 0)
        Precision = 100;
    if (InitialStepSize == 0)
        InitialStepSize = 1;
    if (MaxSwaps < 0)
        MaxSwaps = Dimension;
    if (KickType > Dimension / 2)
        KickType = Dimension / 2;
    if (Runs == 0)
        Runs = 10;
    if (MaxCandidates > Dimension - 1)
        MaxCandidates = Dimension - 1;
    if (ExtraCandidates > Dimension - 1)
        ExtraCandidates = Dimension - 1;
    if (Scale < 1)
        Scale = 1;
    if (SubproblemSize >= Dimension)
        SubproblemSize = Dimension;
    else if (SubproblemSize == 0) {
        if (AscentCandidates > Dimension - 1)
            AscentCandidates = Dimension - 1;
        if (InitialPeriod < 0) {
            InitialPeriod = Dimension / 2;
            if (InitialPeriod < 100)
                InitialPeriod = 100;
        }
        if (Excess < 0)
            Excess = 1.0 / DimensionSaved * Salesmen;
        if (MaxTrials == -1)
            MaxTrials = Dimension;
        HeapMake(Dimension);
    }
    if (POPMUSIC_MaxNeighbors > Dimension - 1)
        POPMUSIC_MaxNeighbors = Dimension - 1;
    if (POPMUSIC_SampleSize > Dimension)
        POPMUSIC_SampleSize = Dimension;
    Depot = &NodeSet[MTSPDepot];
    if (ProblemType == CVRP ||
        ProblemType == CluVRP || ProblemType == SoftCluVRP) {
        Node *N;
        int MinSalesmen;
        if (Capacity <= 0)
            eprintf("CAPACITY not specified");
        TotalDemand = 0;
        N = FirstNode;
        do
            TotalDemand += N->Demand;
        while ((N = N->Suc) != FirstNode);
        MinSalesmen =
            TotalDemand / Capacity + (TotalDemand % Capacity != 0);
        if (Salesmen == 1) {
            Salesmen = MinSalesmen;
            if (Salesmen > Dimension)
                eprintf("CVRP: SALESMEN larger than DIMENSION");
        }
        else if (Salesmen < MinSalesmen)
            eprintf("CVRP: SALESMEN too small to meet demand");
        assert(Salesmen >= 1 && Salesmen <= Dimension);
        if (Salesmen == 1 && ProblemType == CVRP)
            ProblemType = TSP;
        Penalty = ProblemType == CVRP ? Penalty_CVRP : Penalty_CluVRP;
    }
    else if (ProblemType == SOP || ProblemType == M1_PDTSP) {
        Constraint *Con;
        Node *Ni, *Nj;
        int n, k;
        OldDistance = Distance;
        Distance = Distance_SOP;
        if (ProblemType == M1_PDTSP) {
            for (i = 2; i < Dim; i++) {
                Ni = &NodeSet[i];
                for (k = n = 0; k < DemandDimension; k++) {
                    n = Ni->M_Demand[k];
                    if (n >= 0)
                        continue;
                    for (j = 2; j < Dim; j++) {
                        if (j == i)
                            continue;
                        Nj = &NodeSet[j];
                        if (Nj->M_Demand[k] == -n) {
                            Ni->C[j] = -1;
                            break;
                        }
                    }
                }
            }
        }
        for (j = 2; j < Dim; j++) {
            Nj = &NodeSet[j];
            for (i = 2; i < Dim; i++) {
                if (i != j && Nj->C[i] == -1) {
                    Ni = &NodeSet[i];
                    Con = (Constraint *)malloc(sizeof(Constraint));
                    Con->t1 = Ni;
                    Con->t2 = Nj;
                    Con->Suc = FirstConstraint;
                    FirstConstraint = Con;
                    Con->Next = Ni->FirstConstraint;
                    Ni->FirstConstraint = Con;
                }
            }
        }
        Salesmen = 1;
        Penalty = ProblemType == SOP ? Penalty_SOP : Penalty_M1_PDTSP;
    }
    if (ProblemType == TSPTW) {
        Salesmen = 1;
        Penalty = Penalty_TSPTW;
    }
    else
        TSPTW_Makespan = 0;
    if (Salesmen > 1) {
        if (Salesmen > Dim && MTSPMinSize > 0)
            eprintf("Too many salesmen/vehicles (>= DIMENSION)");
        MTSP2TSP();
    }
    else if (MTSPMaxSize == -1)
        MTSPMaxSize = Dimension - 1;
    if (ProblemType == STTSP)
        STTSP2TSP();
    if (ProblemType == ACVRP || ProblemType == ADCVRP)
        Penalty = Penalty_ACVRP;
    else if (ProblemType == CCVRP)
        Penalty = Penalty_CCVRP;
    else if (ProblemType == CTSP)
        Penalty = Penalty_CTSP;
    else if (ProblemType == CBTSP)
        Penalty = Penalty_CBTSP;
    else if (ProblemType == CTSP_D)
        Penalty = Penalty_CTSP_D;
    else if (ProblemType == CBnTSP)
        Penalty = Penalty_CBnTSP;
    else if (ProblemType == CVRPTW)
        Penalty = Penalty_CVRPTW;
    else if (ProblemType == KTSP)
        Penalty = Penalty_KTSP;
    else if (ProblemType == MLP)
        Penalty = Penalty_MLP;
    else if (ProblemType == OVRP)
        Penalty = Penalty_OVRP;
    else if (ProblemType == PDTSP)
        Penalty = Penalty_PDTSP;
    else if (ProblemType == PDTSPF)
        Penalty = Penalty_PDTSPF;
    else if (ProblemType == PDTSPL)
        Penalty = Penalty_PDTSPL;
    else if (ProblemType == PDPTW)
        Penalty = Penalty_PDPTW;
    else if (ProblemType == ONE_PDTSP)
        Penalty = Penalty_1_PDTSP;
    else if (ProblemType == M_PDTSP)
        Penalty = Penalty_M_PDTSP;
    else if (ProblemType == M1_PDTSP)
        Penalty = Penalty_M1_PDTSP;
    else if (ProblemType == PTSP)
        Penalty = Penalty_PTSP;
    else if (ProblemType == RCTVRP || ProblemType == RCTVRPTW)
        Penalty = Penalty_RCTVRP;
    else if (ProblemType == TRP)
        Penalty = Penalty_TRP;
    else if (ProblemType == TSPDL)
        Penalty = Penalty_TSPDL;
    else if (ProblemType == TSPPD)
        Penalty = Penalty_TSPPD;
    if (ProblemType == VRPB)
        Penalty = Penalty_VRPB;
    else if (ProblemType == VRPBTW)
        Penalty = Penalty_VRPBTW;
    else if (ProblemType == VRPPD)
        Penalty = Penalty_VRPPD;
    if (BWTSP_B > 0) {
        if (Penalty)
            eprintf("BWTSP not compatible with problem type %s\n", Type);
        ProblemType = BWTSP;
        free(Type);
        // Type = Copy("BWTSP");
        Penalty = Penalty_BWTSP;
        if (BWTSP_L != INT_MAX)
            BWTSP_L *= Scale;
    }
    if (Penalty && (SubproblemSize > 0 || SubproblemTourFile))
        eprintf("Partitioning not implemented for constrained problems");
    Depot->DepotId = 1;
    for (i = Dim + 1; i <= DimensionSaved; i++)
        NodeSet[i].DepotId = i - Dim + 1;
    if (Dimension != DimensionSaved) {
        NodeSet[Depot->Id + DimensionSaved].DepotId = 1;
        for (i = Dim + 1; i <= DimensionSaved; i++)
            NodeSet[i + DimensionSaved].DepotId = i - Dim + 1;
    }
    if (Scale < 1)
        Scale = 1;
    else {
        Node *Ni = FirstNode;
        do {
            Ni->Earliest *= Scale;
            Ni->Latest *= Scale;
            Ni->ServiceTime *= Scale;
        } while ((Ni = Ni->Suc) != FirstNode);
        ServiceTime *= Scale;
        RiskThreshold *= Scale;
        if (DistanceLimit != DBL_MAX)
            DistanceLimit *= Scale;
    }
    if (ServiceTime != 0) {
        for (i = 1; i <= Dim; i++)
            NodeSet[i].ServiceTime = ServiceTime;
        Depot->ServiceTime = 0;
    }
    if (CostMatrix == 0 && Dimension <= MaxMatrixDimension &&
        Distance != 0 && Distance != Distance_1
        && Distance != Distance_EXPLICIT
        && Distance != Distance_LARGE && Distance != Distance_ATSP
        && Distance != Distance_MTSP && Distance != Distance_SPECIAL) {
        Node *Ni, *Nj;
        CostMatrix = (int *)calloc((size_t)Dim * (Dim - 1) / 2, sizeof(int));
        Ni = FirstNode->Suc;
        do {
            Ni->C =
                &CostMatrix[(size_t)(Ni->Id - 1) * (Ni->Id - 2) / 2] - 1;
            if (ProblemType != HPP || Ni->Id <= Dim)
                for (Nj = FirstNode; Nj != Ni; Nj = Nj->Suc)
                    Ni->C[Nj->Id] = Fixed(Ni, Nj) ? 0 : Distance(Ni, Nj);
            else
                for (Nj = FirstNode; Nj != Ni; Nj = Nj->Suc)
                    Ni->C[Nj->Id] = 0;
        } while ((Ni = Ni->Suc) != FirstNode);
        c = 0;
        WeightType = EXPLICIT;
    }
    if (ProblemType == TSPTW ||
        ProblemType == CVRPTW || ProblemType == VRPBTW ||
        ProblemType == PDPTW || ProblemType == RCTVRPTW ||
        ProblemType == KTSP) {
        M = INT_MAX / 2 / Precision;
        for (i = 1; i <= Dim; i++) {
            Node *Ni = &NodeSet[i];
            for (j = 1; j <= Dim; j++) {
                Node *Nj = &NodeSet[j];
                if (Ni != Nj &&
                    Ni->Earliest + Ni->ServiceTime + Ni->C[j] > Nj->Latest)
                    Ni->C[j] = M;
            }
        }
        if (ProblemType == TSPTW) {
            for (i = 1; i <= Dim; i++)
                for (j = 1; j <= Dim; j++)
                    if (j != i)
                        NodeSet[i].C[j] += NodeSet[i].ServiceTime;
        }
    }
    C = WeightType == EXPLICIT ? C_EXPLICIT : C_FUNCTION;
    D = WeightType == EXPLICIT ? D_EXPLICIT : D_FUNCTION;
    if (ProblemType != CVRP && ProblemType != CVRPTW &&
        ProblemType != CTSP && ProblemType != STTSP &&
        ProblemType != CBTSP && ProblemType != CBnTSP &&
        ProblemType != TSP && ProblemType != ATSP &&
        ProblemType != KTSP && ProblemType != CluVRP &&
        ProblemType != SoftCluVRP) {
        M = INT_MAX / 2 / Precision;
        for (i = Dim + 1; i <= DimensionSaved; i++) {
            for (j = 1; j <= DimensionSaved; j++) {
                if (j == i)
                    continue;
                if (j == MTSPDepot || j > Dim)
                    NodeSet[i].C[j] = NodeSet[MTSPDepot].C[j] = M;
                NodeSet[i].C[j] = NodeSet[MTSPDepot].C[j];
                NodeSet[j].C[i] = NodeSet[j].C[MTSPDepot];
            }
        }
        if (ProblemType == CCVRP || ProblemType == OVRP)
            for (i = 1; i <= Dim; i++)
                NodeSet[i].C[MTSPDepot] = 0;
    }
    if (Precision > 1 && CostMatrix) {
        for (i = 2; i <= Dim; i++) {
            Node *N = &NodeSet[i];
            for (j = 1; j < i; j++)
                if (N->C[j] * Precision / Precision != N->C[j])
                    eprintf("PRECISION (= %d) is too large", Precision);
        }
    }
    if (SubsequentMoveType == 0) {
        // BM k-opt move; k==5
        SubsequentMoveType = MoveType;
        SubsequentMoveTypeSpecial = MoveTypeSpecial;
    }
    K = MoveType >= SubsequentMoveType || !SubsequentPatching ?
        MoveType : SubsequentMoveType;
    if (PatchingC > K)
        PatchingC = K;
    if (PatchingA > 1 && PatchingA >= PatchingC)
        PatchingA = PatchingC > 2 ? PatchingC - 1 : 1;
    if (NonsequentialMoveType == -1 ||
        NonsequentialMoveType > K + PatchingC + PatchingA - 1)
        NonsequentialMoveType = K + PatchingC + PatchingA - 1;
    if (PatchingC >= 1) {
        BestMove = BestSubsequentMove = BestKOptMove;
        if (!SubsequentPatching && SubsequentMoveType <= 5) {
            MoveFunction BestOptMove[] =
            {0, 0, Best2OptMove, Best3OptMove,
            Best4OptMove, Best5OptMove
            };
            BestSubsequentMove = BestOptMove[SubsequentMoveType];
        }
    }
    else {
        MoveFunction BestOptMove[] = {0, 0, Best2OptMove, Best3OptMove,
            Best4OptMove, Best5OptMove
        };
        BestMove = MoveType <= 5 ? BestOptMove[MoveType] : BestKOptMove;
        BestSubsequentMove = SubsequentMoveType <= 5 ?
            BestOptMove[SubsequentMoveType] : BestKOptMove;
    }
    if (MoveTypeSpecial)
        BestMove = BestSpecialOptMove;
    if (SubsequentMoveTypeSpecial)
        BestSubsequentMove = BestSpecialOptMove;
    if (ProblemType == HCP || ProblemType == HPP)
        MaxCandidates = 0;
    if (TraceLevel >= 1) {
        printff("done\n");
        PrintParameters();
    }
    else
        // printff("PROBLEM_FILE = %s\n",
                // ProblemFileName ? ProblemFileName : "");
    // fclose(ProblemFile);
    if (InitialTourFileName)
        ReadTour(InitialTourFileName, &InitialTourFile);
    if (InputTourFileName)
        ReadTour(InputTourFileName, &InputTourFile);
    if (SubproblemTourFileName && SubproblemSize > 0)
        ReadTour(SubproblemTourFileName, &SubproblemTourFile);
    if (MergeTourFiles >= 1) {
        free(MergeTourFile);
        MergeTourFile = (FILE **)malloc(MergeTourFiles * sizeof(FILE *));
        for (i = 0; i < MergeTourFiles; i++)
            ReadTour(MergeTourFileName[i], &MergeTourFile[i]);
    }
    free(LastLine);
    LastLine = 0;
}

static void WriteTour_embeded(char *FileName, int *Tour, GainType Cost) {
    FILE *TourFile = stdout;
    int i, j, k, n, Forward, a, b;
    // char *FullFileName;
    time_t Now;

    if (CurrentPenalty != 0 && MTSPObjective == -1 &&
        ProblemType != CCVRP && ProblemType != TRP &&
        ProblemType != CBTSP && ProblemType != CBnTSP &&
        ProblemType != KTSP && ProblemType != PTSP &&
        ProblemType != MLP)
        return;
    // if (FileName == 0)
    //     return;
    // FullFileName = FullName(FileName, Cost);
    Now = time(&Now);
    // if (TraceLevel >= 1)
        // printff("Writing%s: \"%s\" ... ",
        //         FileName == TourFileName ? " TOUR_FILE" :
        //         FileName == OutputTourFileName ? " OUTPUT_TOUR_FILE" : "",
        //         FullFileName);
    // TourFile = fopen(FullFileName, "w");
    // TourFile = fopen(FullFileName, "w");
    if (CurrentPenalty == 0) {
        fprintf(TourFile, "NAME : %s." GainFormat ".tour\n", Name, Cost);
        fprintf(TourFile, "COMMENT : Length = " GainFormat "\n", Cost);
    }
    else {
        fprintf(TourFile, "NAME : %s." GainFormat "_" GainFormat ".tour\n",
                Name, CurrentPenalty, Cost);
        fprintf(TourFile,
                "COMMENT : Cost = " GainFormat "_" GainFormat "\n",
                CurrentPenalty, Cost);
    }
    fprintf(TourFile, "COMMENT : Found by LKH-3 [Keld Helsgaun] %s",
            ctime(&Now));
    fprintf(TourFile, "TYPE : TOUR\n");
    fprintf(TourFile, "DIMENSION : %d\n", DimensionSaved);
    fprintf(TourFile, "TOUR_SECTION\n");

    n = DimensionSaved;
    for (i = 1; i < n && Tour[i] != MTSPDepot; i++);
    Forward = Asymmetric ||
        Tour[i < n ? i + 1 : 1] < Tour[i > 1 ? i - 1 : Dimension];
    // if (ProblemType == CTSP_D)
    //     Forward = Best_CTSP_D_Direction(Tour);
    int index = 0;
    for (j = 1; j <= n; j++) {
        if ((a = Tour[i]) <= n)
            // fprintf(TourFile, "%d\n",
                    // ProblemType != STTSP ? a : NodeSet[a].OriginalId);
            path[index++] = a;
        if (Forward) {
            if (++i > n)
                i = 1;
        }
        else if (--i < 1)
            i = n;
        // if (ProblemType == STTSP) {
        //     b = Tour[i];
        //     for (k = 0; k < NodeSet[a].PathLength[b]; k++)
        //         fprintf(TourFile, "%d\n", NodeSet[a].Path[b][k]);
        // }
    }
    fprintf(TourFile, "-1\nEOF\n");
    // fclose(TourFile);
    // free(FullFileName);
    // if (TraceLevel >= 1)
        // printff("done\n");
}

int LKH_main() {
    GainType Cost, OldOptimum;
    double Time, LastTime;
    Node *N;
    int i;

    ReadParameters_embeded();
    StartTime = LastTime = GetTime();
    MaxMatrixDimension = 20000;
    MergeWithTour =
        Recombination == GPX2 ? MergeWithTourGPX2 :
        Recombination == CLARIST ? MergeWithTourCLARIST :
        MergeWithTourIPT;
    ReadProblem_embeded();
    if (SubproblemSize > 0) {
        if (DelaunayPartitioning)
            SolveDelaunaySubproblems();
        else if (KarpPartitioning)
            SolveKarpSubproblems();
        else if (KCenterPartitioning)
            SolveKCenterSubproblems();
        else if (KMeansPartitioning)
            SolveKMeansSubproblems();
        else if (RohePartitioning)
            SolveRoheSubproblems();
        else if (MoorePartitioning || SierpinskiPartitioning)
            SolveSFCSubproblems();
        else
            SolveTourSegmentSubproblems();
        return EXIT_SUCCESS;
    }
    AllocateStructures();
    if (ProblemType == TSPTW)
        TSPTW_Reduce();
    if (ProblemType == VRPB || ProblemType == VRPBTW)
        VRPB_Reduce();
    if (ProblemType == PDPTW)
        PDPTW_Reduce();
    CreateCandidateSet();
    InitializeStatistics();

    if (Norm != 0 || Penalty) {
        Norm = 9999;
        BestCost = PLUS_INFINITY;
        BestPenalty = CurrentPenalty = PLUS_INFINITY;
    }
    else {
     /* The ascent has solved the problem! */
        Optimum = BestCost = (GainType)LowerBound;
        UpdateStatistics(Optimum, GetTime() - LastTime);
        RecordBetterTour();
        RecordBestTour();
        CurrentPenalty = PLUS_INFINITY;
        BestPenalty = CurrentPenalty = Penalty ? Penalty() : 0;
        WriteTour_embeded(OutputTourFileName, BestTour, BestCost);
        WriteTour_embeded(TourFileName, BestTour, BestCost);
        Runs = 0;
    }

    /* Find a specified number (Runs) of local optima */

    for (Run = 1; Run <= Runs; Run++) {
        LastTime = GetTime();
        if (LastTime - StartTime >= TotalTimeLimit) {
            if (TraceLevel >= 1)
                printff("*** Time limit exceeded ***\n");
            Run--;
            break;
        }
        Cost = FindTour();      /* using the Lin-Kernighan heuristic */
        if (MaxPopulationSize > 1 && !TSPTW_Makespan) {
            /* Genetic algorithm */
            int i;
            for (i = 0; i < PopulationSize; i++) {
                GainType OldPenalty = CurrentPenalty;
                GainType OldCost = Cost;
                Cost = MergeTourWithIndividual(i);
                if (TraceLevel >= 1 &&
                    (CurrentPenalty < OldPenalty ||
                        (CurrentPenalty == OldPenalty && Cost < OldCost))) {
                    if (CurrentPenalty)
                        printff("  Merged with %d: Cost = " GainFormat "_"
                                GainFormat, i + 1, CurrentPenalty, Cost);
                    else
                        printff("  Merged with %d: Cost = " GainFormat,
                                i + 1, Cost);
                    if (Optimum != MINUS_INFINITY && Optimum != 0) {
                        if (!Penalty ||
                            (ProblemType != CCVRP &&
                                ProblemType != CBTSP &&
                                ProblemType != CBnTSP &&
                                ProblemType != KTSP &&
                                ProblemType != MLP &&
                                ProblemType != PTSP &&
                                ProblemType != TRP &&
                                MTSPObjective != MINMAX &&
                                MTSPObjective != MINMAX_SIZE))
                            printff(", Gap = %0.4f%%",
                                    100.0 * (Cost - Optimum) / Optimum);
                        else
                            printff(", Gap = %0.4f%%",
                                    100.0 * (CurrentPenalty - Optimum) /
                                    Optimum);
                    }
                    printff("\n");
                }
            }
            if (!HasFitness(CurrentPenalty, Cost)) {
                if (PopulationSize < MaxPopulationSize) {
                    AddToPopulation(CurrentPenalty, Cost);
                    if (TraceLevel >= 1)
                        PrintPopulation();
                }
                else if (SmallerFitness(CurrentPenalty, Cost,
                    PopulationSize - 1)) {
                    i = ReplacementIndividual(CurrentPenalty, Cost);
                    ReplaceIndividualWithTour(i, CurrentPenalty, Cost);
                    if (TraceLevel >= 1)
                        PrintPopulation();
                }
            }
        }
        else if (Run > 1 && !TSPTW_Makespan)
            Cost = MergeTourWithBestTour();
        if (CurrentPenalty < BestPenalty ||
            (CurrentPenalty == BestPenalty && Cost < BestCost)) {
            BestPenalty = CurrentPenalty;
            BestCost = Cost;
            RecordBetterTour();
            RecordBestTour();
            WriteTour_embeded(TourFileName, BestTour, BestCost);
        }
        OldOptimum = Optimum;
        if (!Penalty ||
            (ProblemType != CCVRP &&
                ProblemType != CBTSP &&
                ProblemType != CBnTSP &&
                ProblemType != KTSP &&
                ProblemType != MLP &&
                ProblemType != PTSP &&
                ProblemType != TRP &&
                Penalty != Penalty_MTSP_MINMAX &&
                Penalty != Penalty_MTSP_MINMAX_SIZE)) {
            if (CurrentPenalty == 0 && Cost < Optimum)
                Optimum = Cost;
        }
        else if (CurrentPenalty < Optimum)
            Optimum = CurrentPenalty;
        if (Optimum < OldOptimum) {
            printff("*** New OPTIMUM = " GainFormat " ***\n", Optimum);
            if (FirstNode->InputSuc) {
                Node *N = FirstNode;
                while ((N = N->InputSuc = N->Suc) != FirstNode);
            }
        }
        Time = fabs(GetTime() - LastTime);
        UpdateStatistics(Cost, Time);
        if (TraceLevel >= 1 && Cost != PLUS_INFINITY) {
            printff("Run %d: ", Run);
            StatusReport(Cost, LastTime, "");
            printff("\n");
        }
        if (StopAtOptimum && MaxPopulationSize >= 1) {
            if (ProblemType != CCVRP && ProblemType != TRP &&
                ProblemType != CBTSP &&
                ProblemType != CBnTSP &&
                ProblemType != KTSP &&
                ProblemType != MLP &&
                ProblemType != PTSP &&
                MTSPObjective != MINMAX &&
                MTSPObjective != MINMAX_SIZE ?
                CurrentPenalty == 0 && Cost == Optimum :
                CurrentPenalty == Optimum) {
                Runs = Run;
                break;
            }
        }
        IsChild = 0;
        if (PopulationSize >= 2 &&
            (PopulationSize == MaxPopulationSize ||
                Run >= 2 * MaxPopulationSize) && Run < Runs) {
            Node *N;
            int Parent1, Parent2;
            Parent1 = LinearSelection(PopulationSize, 1.25);
            do
                Parent2 = LinearSelection(PopulationSize, 1.25);
            while (Parent2 == Parent1);
            ApplyCrossover(Parent1, Parent2);
            IsChild = 1;
            N = FirstNode;
            do {
                if (ProblemType != HCP && ProblemType != HPP) {
                    int d = C(N, N->Suc);
                    AddCandidate(N, N->Suc, d, INT_MAX);
                    AddCandidate(N->Suc, N, d, INT_MAX);
                }
                N = N->InitialSuc = N->Suc;
            } while (N != FirstNode);
        }
        SRandom(++Seed);
    }
    PrintStatistics();
    if (Salesmen > 1) {
        if (Dimension == DimensionSaved) {
            for (i = 1; i <= Dimension; i++) {
                N = &NodeSet[BestTour[i - 1]];
                (N->Suc = &NodeSet[BestTour[i]])->Pred = N;
            }
        }
        else {
            for (i = 1; i <= DimensionSaved; i++) {
                Node *N1 = &NodeSet[BestTour[i - 1]];
                Node *N2 = &NodeSet[BestTour[i]];
                Node *M1 = &NodeSet[N1->Id + DimensionSaved];
                Node *M2 = &NodeSet[N2->Id + DimensionSaved];
                (M1->Suc = N1)->Pred = M1;
                (N1->Suc = M2)->Pred = N1;
                (M2->Suc = N2)->Pred = M2;
            }
        }
        CurrentPenalty = BestPenalty;
        MTSP_Report(BestPenalty, BestCost);
        MTSP_WriteSolution(MTSPSolutionFileName, BestPenalty, BestCost);
    }
    SINTEF_WriteSolution(SINTEFSolutionFileName, BestCost);
    if (ProblemType == ACVRP ||
        ProblemType == BWTSP ||
        ProblemType == CCVRP ||
        ProblemType == CTSP ||
        ProblemType == CVRP ||
        ProblemType == CVRPTW ||
        ProblemType == MLP ||
        ProblemType == M_PDTSP ||
        ProblemType == M1_PDTSP ||
        MTSPObjective != -1 ||
        ProblemType == ONE_PDTSP ||
        ProblemType == OVRP ||
        ProblemType == PDTSP ||
        ProblemType == PDTSPL ||
        ProblemType == PDPTW ||
        ProblemType == PTSP ||
        ProblemType == RCTVRP ||
        ProblemType == RCTVRPTW ||
        ProblemType == SOP ||
        ProblemType == TRP ||
        ProblemType == TSPTW ||
        ProblemType == VRPB ||
        ProblemType == VRPBTW || ProblemType == VRPPD) {
        printff("Best %s solution:\n", Type);
        CurrentPenalty = BestPenalty;
        SOP_Report(BestCost);
    }
    printff("\n");
    return EXIT_SUCCESS;
}

/**
 * @brief  LKH算法
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t IOScheduleAlgorithmLKH_embeded(const InputParam *input, OutputParam *output) {
    const int MAT_SIZE = input->ioVec.len + 1;
    const Context context = {.input = input};
    // static变量
    ctx = &context;
    path = (int *)calloc(MAT_SIZE, sizeof(int));;

    LKH_main();
    rotate_list(path, MAT_SIZE, MAT_SIZE);

    // 输出Output
    output->len = input->ioVec.len;
    for (size_t r_i = 0, w_i = 0; r_i < MAT_SIZE; r_i++) {
        int index = path[r_i] - 1;
        if (index < output->len) {
            output->sequence[w_i] = input->ioVec.ioArray[index].id;
            // printf("%ld %d %d\n",r_i, index, input->ioVec.ioArray[index].id);
            w_i++;
        }
    }
    free(path);
    return RETURN_OK;
}