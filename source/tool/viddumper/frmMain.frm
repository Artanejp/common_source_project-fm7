VERSION 5.00
Begin VB.Form frmMain 
   BorderStyle     =   1  'å≈íË(é¿ê¸)
   Caption         =   "getrom"
   ClientHeight    =   4260
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   6300
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4260
   ScaleWidth      =   6300
   StartUpPosition =   3  'Windows ÇÃä˘íËíl
   Begin VB.CommandButton cmdGet 
      Caption         =   "Manual"
      Height          =   375
      Left            =   4920
      TabIndex        =   14
      Top             =   3840
      Width           =   1335
   End
   Begin VB.CommandButton cmdCapture 
      Caption         =   "Capture"
      Height          =   375
      Left            =   4920
      TabIndex        =   13
      Top             =   1200
      Width           =   1335
   End
   Begin VB.CommandButton cmdAuto 
      Caption         =   "Stop Auto"
      Height          =   375
      Index           =   1
      Left            =   4920
      TabIndex        =   12
      Top             =   3480
      Visible         =   0   'False
      Width           =   1335
   End
   Begin VB.CommandButton cmdAuto 
      Caption         =   "Start Auto"
      Height          =   375
      Index           =   0
      Left            =   4920
      TabIndex        =   11
      Top             =   3480
      Width           =   1335
   End
   Begin VB.TextBox txtCntY 
      Alignment       =   1  'âEëµÇ¶
      Height          =   270
      Left            =   5280
      TabIndex        =   9
      Text            =   "8"
      Top             =   3120
      Width           =   975
   End
   Begin VB.TextBox txtCntX 
      Alignment       =   1  'âEëµÇ¶
      Height          =   270
      Left            =   5280
      TabIndex        =   8
      Text            =   "8"
      Top             =   2880
      Width           =   975
   End
   Begin VB.CommandButton cmdPos 
      Caption         =   "Auto"
      Height          =   375
      Index           =   2
      Left            =   4920
      TabIndex        =   7
      Top             =   2400
      Width           =   1335
   End
   Begin VB.Timer tmrAuto 
      Enabled         =   0   'False
      Interval        =   200
      Left            =   120
      Top             =   4440
   End
   Begin VB.CommandButton cmdSource 
      Caption         =   "Source"
      Height          =   375
      Left            =   4920
      TabIndex        =   6
      Top             =   840
      Width           =   1335
   End
   Begin VB.CommandButton cmdPos 
      Caption         =   "Right Bottom"
      Height          =   375
      Index           =   1
      Left            =   4920
      TabIndex        =   5
      Top             =   2040
      Width           =   1335
   End
   Begin VB.CommandButton cmdPos 
      Caption         =   "Left Top"
      Height          =   375
      Index           =   0
      Left            =   4920
      TabIndex        =   4
      Top             =   1680
      Width           =   1335
   End
   Begin VB.PictureBox picCapt 
      AutoRedraw      =   -1  'True
      AutoSize        =   -1  'True
      Height          =   3615
      Left            =   0
      ScaleHeight     =   257.376
      ScaleMode       =   0  '’∞ªﬁ∞
      ScaleWidth      =   337.01
      TabIndex        =   3
      Top             =   480
      Width           =   4815
   End
   Begin VB.CommandButton cmdDisconnect 
      Caption         =   "Disconnect"
      Height          =   375
      Left            =   4920
      TabIndex        =   2
      Top             =   480
      Width           =   1335
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect"
      Height          =   375
      Left            =   4920
      TabIndex        =   1
      Top             =   120
      Width           =   1335
   End
   Begin VB.ComboBox cboDevice 
      Height          =   300
      Left            =   0
      Style           =   2  'ƒﬁ€ØÃﬂ¿ﬁ≥› ÿΩƒ
      TabIndex        =   0
      Top             =   120
      Width           =   4815
   End
   Begin VB.Label Label1 
      Caption         =   "Y"
      Height          =   255
      Left            =   5040
      TabIndex        =   15
      Top             =   3120
      Width           =   255
   End
   Begin VB.Label lblCntX 
      Caption         =   "X"
      Height          =   255
      Left            =   5040
      TabIndex        =   10
      Top             =   2880
      Width           =   255
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Declare Sub InitializeDLL Lib "vcap.dll" ()
Private Declare Sub ReleaseDLL Lib "vcap.dll" ()
Private Declare Function GetDeviceNum Lib "vcap.dll" () As Long
Private Declare Sub GetDeviceName Lib "vcap.dll" (ByVal Index As Long, ByVal name As String)
Private Declare Function InitializeDevice Lib "vcap.dll" (ByVal Index As Long) As Long
Private Declare Sub ReleaseDevice Lib "vcap.dll" ()
Private Declare Sub ShowVideoFilterProperty Lib "vcap.dll" ()
Private Declare Function ShowVideoPinProperty Lib "vcap.dll" (ByVal Index As Long) As Long
Private Declare Sub ShowVideoSourceProperty Lib "vcap.dll" ()
Private Declare Sub SetTopMost Lib "vcap.dll" (ByVal status As Long)
Private Declare Sub Capture Lib "vcap.dll" ()

Private mblnConnect As Boolean
Private mblnPrev As Boolean

Private msngX(0 To 3) As Single
Private msngY(0 To 3) As Single

Private Sub cmdAuto_Click(Index As Integer)
    mblnPrev = True
    tmrAuto.Enabled = IIf(Index, False, True)
    cmdAuto(0).Visible = Not cmdAuto(0).Visible
    cmdAuto(1).Visible = Not cmdAuto(1).Visible
End Sub

Private Sub Form_Load()
Dim strName As String
Dim lngDevs As Long
Dim ii As Long

    Call InitializeDLL
    lngDevs = GetDeviceNum()
    
    cboDevice.Clear
    For ii = 1 To lngDevs
        strName = Space(1024)
        Call GetDeviceName(ii, strName)
        cboDevice.AddItem Left$(strName, InStr(strName, Chr$(0)) - 1)
    Next ii
    cboDevice.ListIndex = 0
    mblnConnect = False
    
    Call msubSetUIF

End Sub

Private Sub Form_Unload(Cancel As Integer)
    If mblnConnect Then
        Call ReleaseDevice
    End If
    Call ReleaseDLL
End Sub

Private Sub cmdCapture_Click()
    If mblnConnect Then
        Call Capture
        picCapt.Picture = LoadPicture("c:\capture.bmp")
    End If
End Sub

Private Sub cmdConnect_Click()
Dim lngDevice As Long
    If Not mblnConnect Then
        lngDevice = cboDevice.ListIndex + 1
        If Not InitializeDevice(lngDevice) Then
            mblnConnect = True
            Call msubSetUIF
        End If
    End If
End Sub

Private Sub cmdDisconnect_Click()
    If mblnConnect Then
        Call ReleaseDevice
        mblnConnect = False
        Call msubSetUIF
    End If
End Sub

Private Sub cmdGet_Click()
    Call Capture
    picCapt.Picture = LoadPicture("c:\capture.bmp")
    DoEvents
    
    Call msubGetScreen
End Sub

Private Sub cmdPos_Click(Index As Integer)
    msngX(Index) = msngX(3)
    msngY(Index) = msngY(3)
End Sub

Private Sub cmdSource_Click()
    If mblnConnect Then
        Call ShowVideoFilterProperty
        Call ShowVideoSourceProperty
    End If
End Sub

Private Sub picCapt_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    msngX(3) = X
    msngY(3) = Y
End Sub

Private Sub tmrAuto_Timer()
    If mblnConnect Then
        Call Capture
        picCapt.Picture = LoadPicture("c:\capture.bmp")
        DoEvents
    
        If mfblnGetPoint(msngX(2), msngY(2)) Then
            If Not mblnPrev Then
                Call msubGetScreen
            End If
            mblnPrev = True
        Else
            mblnPrev = False
        End If
    End If
End Sub

Private Sub msubSetUIF()
    cmdConnect.Enabled = Not mblnConnect
    cmdDisconnect.Enabled = mblnConnect
    cmdSource.Enabled = mblnConnect
End Sub

Private Sub msubGetScreen()
Dim fn As Integer
Dim lngVal As Long
Dim lngX As Long
Dim lngY As Long
Dim lngCntX As Long
Dim lngCntY As Long
Dim sngSX As Single
Dim sngSY As Single
Dim sngDX As Single
Dim sngDY As Single
       
    fn = FreeFile
    Open "c:\capture.log" For Append As #fn
    
    lngCntX = Val(txtCntX.Text)
    lngCntY = Val(txtCntY.Text)
    
    sngDX = (msngX(1) - msngX(0)) / (lngCntX + 1)
    sngDY = (msngY(1) - msngY(0)) / (lngCntY - 1)
    sngSX = msngX(0) + sngDX
    sngSY = msngY(0)
    
    For lngY = 1 To lngCntY
        lngVal = 0
        For lngX = 1 To lngCntX
            lngVal = lngVal * 2 + IIf(mfblnGetPoint(sngSX + sngDX * (lngX - 1), sngSY + sngDY * (lngY - 1)), 1, 0)
        Next lngX
        Print #fn, Right$("00000000" & Hex$(lngVal), 8)
    Next lngY
    
    Close #fn

End Sub

Private Function mfblnGetPoint(X As Single, Y As Single) As Boolean
Dim lngCol As Long
    lngCol = picCapt.Point(X, Y)
    mfblnGetPoint = IIf((lngCol And &HC00000) <> 0 And (lngCol And &HC000) <> 0 And (lngCol And &HC0) <> 0, True, False)
End Function
