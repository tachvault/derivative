VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Normal"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

Private Const epsilon = 10 ^ -15
Private Const pi = 3.14159265358979
Private Const invsqr2pi = 0.398942280401433
Private Const tenoversqrttwo = 7.07106781186547
Private icounter As Integer

Private XAbs As Double
Private Exponential As Double
Private build As Double
Private zz As Double
Private z As Double
Private p As Double
Private T As Double
Private Sum As Double

Public Function Bound(Minimum As Double, Variable As Double, Maximum As Double) As Double
  If Variable < Minimum Then
    Bound = Minimum
  ElseIf Variable > Maximum Then
    Bound = Maximum
  Else
    Bound = Variable
  End If
End Function

Public Function Max(First_No As Double, Second_No As Double) As Double
  If First_No > Second_No Then
    Max = First_No
  Else
    Max = Second_No
  End If
End Function

Public Function Min(First_No As Double, Second_No As Double) As Double
  If First_No > Second_No Then
    Min = Second_No
  Else
    Min = First_No
  End If
End Function


Function Cumnorm(x As Double) As Double
  XAbs = Abs(x)
  If XAbs > 37 Then
    Cumnorm = 0
  Else
    Exponential = Exp(-XAbs ^ 2 / 2)
    If XAbs < tenoversqrttwo Then
      build = 3.52624965998911E-02 * XAbs + 0.700383064443688
      build = build * XAbs + 6.37396220353165
      build = build * XAbs + 33.912866078383
      build = build * XAbs + 112.079291497871
      build = build * XAbs + 221.213596169931
      build = build * XAbs + 220.206867912376
      Cumnorm = Exponential * build
      build = 8.83883476483184E-02 * XAbs + 1.75566716318264
      build = build * XAbs + 16.064177579207
      build = build * XAbs + 86.7807322029461
      build = build * XAbs + 296.564248779674
      build = build * XAbs + 637.333633378831
      build = build * XAbs + 793.826512519948
      build = build * XAbs + 440.413735824752
      Cumnorm = Cumnorm / build
    Else
      build = XAbs + 0.65
      build = XAbs + 4 / build
      build = XAbs + 3 / build
      build = XAbs + 2 / build
      build = XAbs + 1 / build
      Cumnorm = Exponential / build / 2.506628274631
    End If
  End If
  If x > 0 Then Cumnorm = 1 - Cumnorm
End Function

Public Function NPrime(x As Double) As Double
  'derivative of cumulative normal distribution
  NPrime = Exp(-0.5 * x ^ 2) * invsqr2pi
End Function

Public Function NPrimePrime(x As Double) As Double
  'second derivative of cumulative normal distribution
  NPrimePrime = -x * NPrime(x)
End Function

Public Function CumNormInverse(y As Double) As Double
  'Moro method
  z = y - 0.5
  If Abs(z) < 0.42 Then
    zz = z ^ 2
    zz = z * (((-25.44106049637 * zz + 41.39119773534) * zz + -18.61500062529) * zz + 2.50662823884) / _
    ((((3.13082909833 * zz + -21.06224101826) * zz + 23.08336743743) * zz + -8.4735109309) * zz + 1)
  Else
    If z > 0 Then zz = Log(-Log(1 - y))
    If z <= 0 Then zz = Log(-Log(y))
    build = 2.888167364E-07 + zz * 3.960315187E-07
    build = 3.21767881768E-05 + zz * build
    build = 3.951896511919E-04 + zz * build
    build = 3.8405729373609E-03 + zz * build
    build = 2.76438810333863E-02 + zz * build
    build = 0.160797971491821 + zz * build
    build = 0.976169019091719 + zz * build
    zz = 0.337475482272615 + zz * build
    If z <= 0 Then zz = -zz
  End If
  CumNormInverse = zz
End Function

Public Function CumNorm6(x As Double) As Double
  'Abramowitz and Stegun - 6dp accuracy.
  
  If x = 0 Then
    CumNorm6 = 0.5
  Else
    T = 1 / (1 + 0.2316419 * Abs(x))
    Sum = 0.31938153 * T - 0.356563782 * T ^ 2 + 1.781477937 * T ^ 3 - 1.821255978 * T ^ 4 + 1.330274429 * T ^ 5
    
    If x < 0 Then
       CumNorm6 = invsqr2pi * Exp(-x ^ 2 / 2) * Sum
    Else
      CumNorm6 = 1 - invsqr2pi * Exp(-x ^ 2 / 2) * Sum
    End If
  End If
  
End Function

Public Function BiVarCumNorm(x As Double, y As Double, Correlation As Double) As Double
'
'     A function for computing bivariate normal probabilities.
'
'       Alan Genz
'       Department of Mathematics
'       Washington State University
'       Pullman, WA 99164-3113
'       Email : alangenz@wsu.edu
'
'    This function is based on the method described by
'        Drezner, Z and G.O. Wesolowsky, (1989),
'        On the computation of the bivariate normal integral,
'        Journal of Statist. Comput. Simul. 35, pp. 101-107,
'    with major modifications for double precision, and for |R| close to 1.

Dim i As Integer, ISs As Integer, LG As Integer, NG As Integer
Dim XX(10, 3) As Double, W(10, 3) As Double
Dim h As Double, k As Double, hk As Double, hs As Double, BVN As Double, Ass As Double, asr As Double, sn As Double
Dim A As Double, B As Double, bs As Double, C As Double, d As Double
Dim xs As Double, rs As Double

W(1, 1) = 0.17132449237917
XX(1, 1) = -0.932469514203152
W(2, 1) = 0.360761573048138
XX(2, 1) = -0.661209386466265
W(3, 1) = 0.46791393457269
XX(3, 1) = -0.238619186083197

W(1, 2) = 4.71753363865118E-02
XX(1, 2) = -0.981560634246719
W(2, 2) = 0.106939325995318
XX(2, 2) = -0.904117256370475
W(3, 2) = 0.160078328543346
XX(3, 2) = -0.769902674194305
W(4, 2) = 0.203167426723066
XX(4, 2) = -0.587317954286617
W(5, 2) = 0.233492536538355
XX(5, 2) = -0.36783149899818
W(6, 2) = 0.249147045813403
XX(6, 2) = -0.125233408511469

W(1, 3) = 1.76140071391521E-02
XX(1, 3) = -0.993128599185095
W(2, 3) = 4.06014298003869E-02
XX(2, 3) = -0.963971927277914
W(3, 3) = 6.26720483341091E-02
XX(3, 3) = -0.912234428251326
W(4, 3) = 8.32767415767048E-02
XX(4, 3) = -0.839116971822219
W(5, 3) = 0.10193011981724
XX(5, 3) = -0.746331906460151
W(6, 3) = 0.118194531961518
XX(6, 3) = -0.636053680726515
W(7, 3) = 0.131688638449177
XX(7, 3) = -0.510867001950827
W(8, 3) = 0.142096109318382
XX(8, 3) = -0.37370608871542
W(9, 3) = 0.149172986472604
XX(9, 3) = -0.227785851141645
W(10, 3) = 0.152753387130726
XX(10, 3) = -7.65265211334973E-02
      
If Abs(Correlation) < 0.3 Then
  NG = 1
  LG = 3
ElseIf Abs(Correlation) < 0.75 Then
  NG = 2
  LG = 6
Else
  NG = 3
  LG = 10
End If
      
h = -x
k = -y
hk = h * k
BVN = 0
      
If Abs(Correlation) < 0.925 Then
  If Abs(Correlation) > 0 Then
    hs = (h * h + k * k) / 2
    asr = ArcSin(Correlation)
    For i = 1 To LG
      For ISs = -1 To 1 Step 2
        sn = Sin(asr * (ISs * XX(i, NG) + 1) / 2)
        BVN = BVN + W(i, NG) * Exp((sn * hk - hs) / (1 - sn * sn))
      Next ISs
    Next i
    BVN = BVN * asr / (4 * pi)
  End If
  BVN = BVN + Cumnorm(-h) * Cumnorm(-k)
Else
  If Correlation < 0 Then
    k = -k
    hk = -hk
  End If
  If Abs(Correlation) < 1 Then
    Ass = (1 - Correlation) * (1 + Correlation)
    A = Sqr(Ass)
    bs = (h - k) ^ 2
    C = (4 - hk) / 8
    d = (12 - hk) / 16
    asr = -(bs / Ass + hk) / 2
    If asr > -100 Then BVN = A * Exp(asr) * (1 - C * (bs - Ass) * (1 - d * bs / 5) / 3 + C * d * Ass * Ass / 5)
    If -hk < 100 Then
      B = Sqr(bs)
      BVN = BVN - Exp(-hk / 2) * Sqr(2 * pi) * Cumnorm(-B / A) * B * (1 - C * bs * (1 - d * bs / 5) / 3)
    End If
    A = A / 2
    For i = 1 To LG
      For ISs = -1 To 1 Step 2
        xs = (A * (ISs * XX(i, NG) + 1)) ^ 2
        rs = Sqr(1 - xs)
        asr = -(bs / xs + hk) / 2
        If asr > -100 Then
           BVN = BVN + A * W(i, NG) * Exp(asr) * (Exp(-hk * (1 - rs) / (2 * (1 + rs))) / rs - (1 + C * xs * (1 + d * xs)))
        End If
      Next ISs
    Next i
    BVN = -BVN / (2 * pi)
  End If
  If Correlation > 0 Then
    BVN = BVN + Cumnorm(-Max(h, k))
  Else
    BVN = -BVN
    If k > h Then BVN = BVN + Cumnorm(k) - Cumnorm(h)
  End If
End If
BiVarCumNorm = BVN

End Function

Public Function TriVarCumNorm(x As Double, y As Double, z As Double, correl12 As Double, correl13 As Double, correl23 As Double) As Double
  
  Dim limit(3) As Double, Correlation(3, 3) As Double
  Dim i As Integer, j As Integer, k As Integer, vector As Integer
  Dim lower As Double, upper As Double, Kronrodint As Double, Kronroderr As Double, FunSum As Double
  Dim ke(10) As Double, kf(10) As Double
  Dim A(10) As Double, B(10) As Double
  Dim Fin As Double, Err As Double
  Dim WG(5) As Double, WGK(11) As Double, XGK(11) As Double
  Dim Halflength As Double, Centre As Double, fc As Double, Resltk As Double, Resltg As Double
  Dim Abscis As Double
  
  limit(1) = x
  limit(2) = y
  limit(3) = z
  Correlation(2, 1) = correl12
  Correlation(3, 1) = correl13
  Correlation(3, 2) = correl23
  
  If Abs(Correlation(2, 1)) > Abs(Correlation(3, 1)) Then
    limit(2) = z
    limit(3) = y
    Correlation(2, 1) = correl13
    Correlation(3, 1) = correl12
  End If
  If Abs(Correlation(3, 1)) > Abs(Correlation(3, 2)) Then
    limit(1) = limit(2)
    limit(2) = x
    Correlation(3, 2) = Correlation(3, 1)
    Correlation(3, 1) = correl23
  End If
  TriVarCumNorm = 0
  If Abs(limit(1)) + Abs(limit(2)) + Abs(limit(3)) < epsilon Then
    TriVarCumNorm = (1 + (ArcSin(Correlation(2, 1)) + ArcSin(Correlation(3, 1)) + ArcSin(Correlation(3, 2))) / ArcSin(1)) / 8
  ElseIf Abs(Correlation(2, 1)) + Abs(Correlation(3, 1)) < epsilon Then
    TriVarCumNorm = Cumnorm(limit(1)) * BiVarCumNorm(limit(2), limit(3), Correlation(3, 2))
  ElseIf Abs(Correlation(3, 1)) + Abs(Correlation(3, 2)) < epsilon Then
    TriVarCumNorm = Cumnorm(limit(3)) * BiVarCumNorm(limit(1), limit(2), Correlation(2, 1))
  ElseIf Abs(Correlation(2, 1)) + Abs(Correlation(3, 2)) < epsilon Then
    TriVarCumNorm = Cumnorm(limit(2)) * BiVarCumNorm(limit(1), limit(3), Correlation(3, 1))
  ElseIf 1 - Correlation(3, 2) < epsilon Then
    TriVarCumNorm = BiVarCumNorm(limit(1), Min(limit(2), limit(3)), Correlation(2, 1))
  ElseIf Correlation(3, 2) + 1 < epsilon Then
    If limit(2) > -limit(3) Then
      TriVarCumNorm = BiVarCumNorm(limit(1), limit(2), Correlation(2, 1)) - BiVarCumNorm(limit(1), -limit(3), Correlation(2, 1))
    Else
      TriVarCumNorm = 0
    End If
  Else
    TriVarCumNorm = BiVarCumNorm(limit(2), limit(3), Correlation(3, 2)) * Cumnorm(limit(1))
    'One Dimensional Globally Adaptive Integration Function
    GoSub Kronroddata
    k = 1
    j = 1
    A(1) = 0
    B(1) = 1
    Do
      j = j + 1
      B(j) = B(k)
      A(j) = (A(k) + B(k)) / 2
      B(k) = A(j)
      lower = A(k)
      upper = B(k)
      GoSub Kronrod
      kf(k) = Kronrodint
      ke(k) = Kronroderr
      lower = A(j)
      upper = B(j)
      GoSub Kronrod
      kf(j) = Kronrodint
      ke(j) = Kronroderr
      Err = 0
      Fin = 0
      For i = 1 To j
        If ke(i) > ke(k) Then k = i
        Fin = Fin + kf(i)
        Err = Err + ke(i) ^ 2
      Next i
      Err = Sqr(Err)
    Loop While 4 * Err > epsilon And j < 10
    TriVarCumNorm = TriVarCumNorm + Fin / (4 * ArcSin(1))
  End If
  TriVarCumNorm = Bound(0, TriVarCumNorm, 1)
Exit Function

Kronroddata:
  WG(0) = 0.272925086777901
  WG(1) = 5.56685671161745E-02
  WG(2) = 0.125580369464905
  WG(3) = 0.186290210927735
  WG(4) = 0.233193764591991
  WG(5) = 0.262804544510248
  WGK(0) = 0.136577794711118
  WGK(1) = 9.76544104596129E-03
  WGK(2) = 2.71565546821044E-02
  WGK(3) = 4.58293785644267E-02
  WGK(4) = 6.30974247503748E-02
  WGK(5) = 7.86645719322276E-02
  WGK(6) = 9.29530985969007E-02
  WGK(7) = 0.105872074481389
  WGK(8) = 0.116739502461047
  WGK(9) = 0.125158799100319
  WGK(10) = 0.131280684229806
  WGK(11) = 0.135193572799885
  XGK(0) = 0
  XGK(1) = 0.996369613889543
  XGK(2) = 0.978228658146057
  XGK(3) = 0.941677108578068
  XGK(4) = 0.887062599768095
  XGK(5) = 0.816057456656221
  XGK(6) = 0.730152005574049
  XGK(7) = 0.630599520161965
  XGK(8) = 0.519096129206812
  XGK(9) = 0.397944140952378
  XGK(10) = 0.269543155952345
  XGK(11) = 0.136113000799362
Return

Kronrod:
  'Kronrod Rule on interval [lower,upper]
  Halflength = (upper - lower) / 2
  Centre = (lower + upper) / 2
  fc = Tvtmfn(Centre, limit(1), limit(2), limit(3), Correlation(2, 1), Correlation(3, 1), Correlation(3, 2))
  Resltg = fc * WG(0)
  Resltk = fc * WGK(0)
  For vector = 1 To 11
    Abscis = Halflength * XGK(vector)
    FunSum = Tvtmfn(Centre - Abscis, limit(1), limit(2), limit(3), Correlation(2, 1), Correlation(3, 1), Correlation(3, 2)) + Tvtmfn(Centre + Abscis, limit(1), limit(2), limit(3), Correlation(2, 1), Correlation(3, 1), Correlation(3, 2))
    Resltk = Resltk + WGK(vector) * FunSum
    If vector Mod 2 = 0 Then Resltg = Resltg + WG(vector / 2) * FunSum
  Next vector
  Kronrodint = Resltk * Halflength
  Kronroderr = Abs(Resltg - Resltk) * Halflength
Return
  
End Function

Private Function ArcSin(x As Double) As Double
  If Abs(x) = 1 Then
    ArcSin = Sgn(x) * pi / 2
  Else
    ArcSin = Atn(x / Sqr(1 - x ^ 2))
  End If
End Function

Private Function Tvtmfn(x As Double, l1 As Double, l2 As Double, l3 As Double, correl21 As Double, correl31 As Double, correl32 As Double) As Double
  'Computes Plackett formula integrands
  Dim rua As Double, rub As Double, ruc As Double, ar As Double, ee As Double
  Dim y(1 To 4) As Double
  rua = ArcSin(correl21)
  rub = ArcSin(correl31)
  y(1) = Sin(rua * x)
  y(2) = Cos(rua * x) ^ 2
  y(3) = Sin(rub * x)
  y(4) = Cos(rub * x) ^ 2
  If Abs(rua) > 0 Then
    Tvtmfn = Tvtmfn + rua * Pntgnd(l1, l2, l3, y(3), correl32, y(1), y(2))
  End If
  If Abs(rub) > 0 Then
    Tvtmfn = Tvtmfn + rub * Pntgnd(l1, l3, l2, y(1), correl32, y(3), y(4))
  End If
End Function

Private Function Pntgnd(ba As Double, bb As Double, bc As Double, ra As Double, rb As Double, r As Double, rr As Double)
  'Computes Plackett formula integrand
  Dim dt As Double, bt As Double, ft As Double
  dt = rr * (rr - (ra - rb) ^ 2 - 2 * ra * rb * (1 - r))
  If dt > 0 Then
    bt = (bc * rr + ba * (r * rb - ra) + bb * (r * ra - rb)) / Sqr(dt)
    ft = (ba - r * bb) ^ 2 / rr + bb ^ 2
    If bt > -10 And ft < 100 Then
        Pntgnd = Exp(-ft / 2)
       If bt < 10 Then Pntgnd = Pntgnd * Cumnorm(bt)
    End If
  End If
End Function



