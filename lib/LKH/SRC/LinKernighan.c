#include "Segment.h"
#include "LKH.h"
#include "Hashing.h"
#include "Sequence.h"
#include "BIT.h"

/*
 * The LinKernighan function seeks to improve a tour by sequential
 * and non-sequential edge exchanges.
 *
 * The function returns the cost of the resulting tour.
 */

GainType LinKernighan()
{
    //Cost:路径总成本
    //Gain:存储在每次交换边后，路径总成本的改善（即增益）。如果增益为正，意味着找到了一条更优路径。
    //G0:初始边t1到t2的成本
    GainType Cost, Gain, G0;
    //X2：用于选择t1的两个邻居节点之一，可能值为1或2
    //it：迭代次数计数器，通常用于记录当前算法进行的迭代次数
    int X2, i, it = 0;
    //t1：当前处理的节点，即边交换操作的起点
    //t2: t1的邻居节点，即与t1进行交换的另一端节点。
    //SUCt1：t1的后继节点，表示在当前路径中的t1的下一个节点。
    Node *t1, *t2, *SUCt1;
    //Nt1：候选边集中的一个指针，表示t1的候选集节点
    Candidate *Nt1;
    //S：段（segment）结构的指针，用于对节点进行分段处理
    //SS：超级段（super-segment）的指针，多个段可以组成超级段，类似于一种分组的组织方式
    Segment *S;
    SSegment *SS;
    //EntryTime：记录函数执行开始时的时间戳
    double EntryTime = GetTime();

    //Cost：设置为0，表示当前路径的初始成本。后续将根据路径的边交换操作更新这个值
    Cost = 0;
    //Reversed：设置为0，表示当前没有段被反转
    Reversed = 0;
    S = FirstSegment;
    i = 0;
    //遍历所有段，直到回到FirstSegment，确保所有段都被初始化
    do {
        S->Size = 0;
        S->Rank = ++i;
        S->Reversed = 0;
        S->First = S->Last = 0;
    }
    while ((S = S->Suc) != FirstSegment);
    //遍历所有超级段，确保它们都被初始化。
    SS = FirstSSegment;
    i = 0;
    do {
        SS->Size = 0;
        SS->Rank = ++i;
        SS->Reversed = 0;
        SS->First = SS->Last = 0;
    }
    while ((SS = SS->Suc) != FirstSSegment);
    //初始化活跃节点链表的首尾指针为0，表示没有活跃节点。
    FirstActive = LastActive = 0;
    //初始化交换计数为0，后续在路径改进过程中将根据实际的边交换操作更新这个值。
    Swaps = 0;

    /* Compute the cost of the initial tour, Cost.
       Compute the corresponding hash value, Hash.
       Initialize the segment list.
       Make all nodes "active" (so that they can be used as t1). */
    Cost = 0;
    Hash = 0;
    i = 0;
    //将t1指向第一个节点，开始遍历所有节点
    t1 = FirstNode;
    do {
        //t1是当前处理的节点 t2是t1的后继结点
        t2 = t1->OldSuc = t1->Suc;
        //确保t2存在
        assert(t2);
        t1->OldPred = t1->Pred;
        //为每个节点分配唯一排序号
        t1->Rank = ++i;
        //计算 t1 到 t2 的边的成本，并更新总成本。
        Cost += (t1->SucCost = t2->PredCost = C(t1, t2)) - t1->Pi - t2->Pi;
        //计算路径的哈希值
        Hash ^= Rand[t1->Id] * Rand[t2->Id];
        t1->Cost = INT_MAX;
        //遍历 t1 的候选集，找到不与当前路径相连且成本最低的边。
        for (Nt1 = t1->CandidateSet; (t2 = Nt1->To); Nt1++)
            if (t2 != t1->Pred && t2 != t1->Suc && Nt1->Cost < t1->Cost)
                t1->Cost = Nt1->Cost;
        // 设置父段为S
        t1->Parent = S;
        S->Size++;
        if (S->Size == 1)
            S->First = t1;
        S->Last = t1;
        if (SS->Size == 0)
            SS->First = S;
        S->Parent = SS;
        SS->Last = S;
        //当当前段的大小达到预设的GroupSize时，移动到下一个段，并更新超级段的大小。
        if (S->Size == GroupSize) {
            S = S->Suc;
            SS->Size++;
            if (SS->Size == SGroupSize)
                SS = SS->Suc;
        }
        t1->OldPredExcluded = t1->OldSucExcluded = 0;
        t1->Next = 0;
        //根据不同的条件决定是否激活节点t1，以便在路径改进时可以使用这个节点。
        //Trial 为 1 时会激活所有节点
        if (KickType == 0 || Kicks == 0 || Trial == 1 ||
        //如果节点 t1 的前驱或后继不在最佳路径中，则激活 t1，这可能是为了探索其他路径的可能性
            !InBestTour(t1, t1->Pred) || !InBestTour(t1, t1->Suc))
            Activate(t1);
    }
    while ((t1 = t1->Suc) != FirstNode);
    if (S->Size < GroupSize)
        SS->Size++;
    //缩放成本值，以便进行比较
    Cost /= Precision;
    //如果启用了时间窗口（TSPTW），则计算当前路径的完工时间
    if (TSPTW_Makespan)
        Cost = TSPTW_CurrentMakespanCost = TSPTW_MakespanCost();
    //计算当前罚金，根据是否定义了 Penalty 函数来计算当前的罚金    
    CurrentPenalty = PLUS_INFINITY;
    CurrentPenalty = Penalty ? Penalty() : 0;
    /*
    根据跟踪级别 TraceLevel，决定是否报告当前状态。
    如果 TraceLevel 达到一定程度或当前的罚金和成本优于之前的最好解，
    就调用 StatusReport 函数进行记录。
    这用于调试和分析算法的执行过程。
    */
    if (TraceLevel >= 3 ||
        (TraceLevel == 2 &&
         (CurrentPenalty < BetterPenalty ||
          (CurrentPenalty == BetterPenalty && Cost < BetterCost))))
        StatusReport(Cost, EntryTime, "");
    //标记前驱和后继的成本可用
    PredSucCostAvailable = 1;
    //更新当前状态或标记
    BIT_Update();

    /* Loop as long as improvements are found */
    //外层循环在找到改进时持续运行，直到没有更多的改善（即罚金增益和收益均为零）。
    do {
        /* Choose t1 as the first "active" node */
        while ((t1 = RemoveFirstActive())) {
            //检查算法是否超过了预设的时间限制
            if (GetTime() - EntryTime >= TimeLimit ||
                GetTime() - StartTime >= TotalTimeLimit) {
                if (TraceLevel >= 1)
                    printff("*** Time limit exceeded");
                goto End_LinKernighan;
            }
            /* t1 is now "passive" */
            //获取后继节点
            SUCt1 = SUC(t1);
            //如果跟踪级别大于或等于 3，输出调试信息。
            //如果跟踪级别为 2，并且当前是第一次试验（Trial 为 1），也输出信息。
            if ((TraceLevel >= 3 || (TraceLevel == 2 && Trial == 1)) &&
            //当 Dimension 大于或等于 100000，输出每 10000 次迭代。
            // 当 Dimension 大于或等于 10000，输出每 1000 次迭代。
            // 否则，每 100 次迭代输出一次。
                ++it % (Dimension >= 100000 ? 10000 :
                        Dimension >= 10000 ? 1000 : 100) == 0)
                //打印当前迭代次数 it 以及从开始时间 EntryTime 到当前时间的持续时间
                printff("#%d: Time = %0.2f sec.\n",
                        it, fabs(GetTime() - EntryTime));
            /* Choose t2 as one of t1's two neighbors on the tour */
            //循环两次，第一次选择 t1 的前驱 (PRED(t1))，第二次选择后继 (SUC(t1))
            for (X2 = 1; X2 <= 2; X2++) {
                t2 = X2 == 1 ? PRED(t1) : SUCt1;
                //条件跳过：1.判断 t1 和 t2 是否被固定或是常见的组合。
                //2.如果使用限制搜索，并且 t1 和 t2 彼此接近，同时满足试验条件，也跳过。
                if (FixedOrCommon(t1, t2) ||
                    (RestrictedSearch && Near(t1, t2) &&
                     (Trial == 1 ||
                      (Trial > BackboneTrials &&
                       (KickType == 0 || Kicks == 0)))))
                    continue;
                //使用成本函数 C(t1, t2) 计算 t1 到 t2 的成本并存储在 G0 中。
                G0 = C(t1, t2);

                /* Try to find a tour-improving chain of moves */
                //寻找改进链：
                //使用 BestMove 或 BestSubsequentMove 尝试找到改进路径，直到没有更多的改进可用
                do
                    t2 = Swaps == 0 ? BestMove(t1, t2, &G0, &Gain) :
                        BestSubsequentMove(t1, t2, &G0, &Gain);
                while (t2);

                //如果找到的收益（Gain）或惩罚收益（PenaltyGain）大于零，说明找到了改进
                if (PenaltyGain > 0 || Gain > 0) {
                    /* An improvement has been found */
                    //精度检查：确保 Gain 和 Precision 的关系符合预期，以避免数值错误。
#ifdef HAVE_LONG_LONG
                    assert(Gain % Precision == 0);
#else
                    assert(fmod(Gain, Precision) == 0);
#endif
                    //更新成本：从当前成本中减去收益并更新当前惩罚，存储当前的旅游路径，同时更新当前的最大完工成本。
                    Cost -= Gain / Precision;
                    CurrentPenalty -= PenaltyGain;
                    StoreTour();
                    TSPTW_CurrentMakespanCost = Cost;

                    //状态报告：如果达到一定的跟踪级别，输出当前状态信息。
                    if (TraceLevel >= 3 ||
                        (TraceLevel == 2 &&
                         (CurrentPenalty < BetterPenalty ||
                          (CurrentPenalty == BetterPenalty &&
                           Cost < BetterCost))))
                        StatusReport(Cost, EntryTime, "");

                    //哈希检查：检查当前路径是否已经存在于哈希表中，如果存在，结束当前的 Lin-Kernighan 过程。
                    if (HashSearch(HTable, Hash, Cost))
                        goto End_LinKernighan;
                    
                    //重新激活节点：将 t1 标记为“激活”，以便在后续迭代中再次使用，并重置交换计数。
                    /* Make t1 "active" again */
                    Activate(t1);
                    OldSwaps = 0;
                    break;
                }
                OldSwaps = 0;
                RestoreTour();
            }
        }
        if (HashSearch(HTable, Hash, Cost))
            goto End_LinKernighan;
        //插入哈希表：如果当前路径不在哈希表中，则将其插入，记录当前的成本。
        HashInsert(HTable, Hash, Cost);
        /* Try to find improvements using non-sequential 4/5-opt moves */
        PenaltyGain = Gain = 0;
        //如果 Gain23Used 为真，则调用 Gain23() 函数尝试进行4/5-opt 移动。
        //如果返回的收益 Gain 大于0，或者 PenaltyGain 大于0，说明找到了改进。
        if (Gain23Used && ((Gain = Gain23()) > 0 || PenaltyGain > 0)) {
            /* An improvement has been found */
#ifdef HAVE_LONG_LONG
            assert(Gain % Precision == 0);
#else
            assert(fmod(Gain, Precision) == 0);
#endif
            //从当前成本中减去收益，并更新当前惩罚。同时更新最大完工成本 TSPTW_CurrentMakespanCost
            Cost -= Gain / Precision;
            CurrentPenalty -= PenaltyGain;
            TSPTW_CurrentMakespanCost = Cost;
            StoreTour();
            if (TraceLevel >= 3 ||
                (TraceLevel == 2 &&
                 (CurrentPenalty < BetterPenalty ||
                  (CurrentPenalty == BetterPenalty && Cost < BetterCost))))
                StatusReport(Cost, EntryTime, " + ");
            if (HashSearch(HTable, Hash, Cost))
                goto End_LinKernighan;
        }
    }
    while (PenaltyGain > 0 || Gain > 0);
  End_LinKernighan:
    //重置成本可用标志
    PredSucCostAvailable = 0;
    //标准化节点列表
    NormalizeNodeList();
    //标准化段列表
    NormalizeSegmentList();
    Reversed = 0;
    return Cost;
}
