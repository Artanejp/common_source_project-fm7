Option Strict On
Option Explicit On

Public Class Form1
	Private Declare Function PostMessage Lib "user32.dll" Alias "PostMessageA" (ByVal hWnd As IntPtr, ByVal wMsg As Integer, ByVal wParam As Integer, ByVal lParam As IntPtr) As IntPtr
	Private Const WM_KEYDOWN As Integer = &H100
	Private Const WM_KEYUP As Integer = &H101
	Private mblnSemaphore As Boolean = False

	Private Function mfintGetWindowHandle() As IntPtr
		For Each objProcess As Diagnostics.Process In Diagnostics.Process.GetProcesses()
			If objProcess.MainWindowTitle.StartsWith("SHARP MZ-80K/C - ") Then
				Return objProcess.MainWindowHandle
			End If
		Next
		Return IntPtr.Zero
	End Function

	Private Sub msubKeyDown(ByVal vintKeyCode As Integer)
		Dim hWnd As IntPtr = mfintGetWindowHandle()
		If hWnd <> IntPtr.Zero Then
			PostMessage(hWnd, WM_KEYDOWN, vintKeyCode, IntPtr.Zero)
		End If
	End Sub

	Private Sub msubKeyUp(ByVal vintKeyCode As Integer)
		Dim hWnd As IntPtr = mfintGetWindowHandle()
		If hWnd <> IntPtr.Zero Then
			PostMessage(hWnd, WM_KEYUP, vintKeyCode, IntPtr.Zero)
		End If
	End Sub

	Private Sub msubKeyHit(ByVal vintKeyCode As Integer)
		Dim hWnd As IntPtr = mfintGetWindowHandle()
		If hWnd <> IntPtr.Zero Then
			PostMessage(hWnd, WM_KEYDOWN, vintKeyCode, IntPtr.Zero)
			Threading.Thread.Sleep(100)
			PostMessage(hWnd, WM_KEYUP, vintKeyCode, IntPtr.Zero)
		End If
	End Sub

	Private Sub msubUpdateShift(ByVal vblnPrevPressed As Boolean)
		Dim blnPressed As Boolean = (Button45.Checked OrElse Button53.Checked)
		If Not vblnPrevPressed AndAlso blnPressed Then
			msubKeyDown(&H10)
		ElseIf vblnPrevPressed AndAlso Not blnPressed Then
			msubKeyUp(&H10)
		End If
	End Sub

	Private Sub Form1_Load(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Load
		Me.ActiveControl = Nothing
	End Sub

	Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles Me.KeyDown
		If e.KeyCode = Keys.ShiftKey Then
			Dim blnPrevPressed As Boolean = (Button45.Checked OrElse Button53.Checked)
			Try
				mblnSemaphore = True
				Button45.Checked = True
				Button45.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT_PRESSED
				Button53.Checked = True
				Button53.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT_PRESSED
			Catch ex As Exception
			Finally
				mblnSemaphore = False
			End Try
			msubUpdateShift(blnPrevPressed)
		Else
			msubKeyDown(e.KeyCode)
		End If
	End Sub

	Private Sub Form1_KeyUp(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles Me.KeyUp
		If e.KeyCode = Keys.ShiftKey Then
			Dim blnPrevPressed As Boolean = (Button45.Checked OrElse Button53.Checked)
			Try
				mblnSemaphore = True
				Button45.Checked = False
				Button45.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT
				Button53.Checked = False
				Button53.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT
			Catch ex As Exception
			Finally
				mblnSemaphore = False
			End Try
			msubUpdateShift(blnPrevPressed)
		ElseIf e.KeyCode = Keys.Up OrElse e.KeyCode = Keys.Down OrElse e.KeyCode = Keys.Left OrElse e.KeyCode = Keys.Right Then
			msubKeyHit(e.KeyCode)
		Else
			msubKeyUp(e.KeyCode)
		End If
	End Sub

	Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click
		msubKeyHit(Asc("1"c))
	End Sub

	Private Sub Button2_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button2.Click
		msubKeyHit(Asc("2"c))
	End Sub

	Private Sub Button3_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button3.Click
		msubKeyHit(Asc("3"c))
	End Sub

	Private Sub Button4_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button4.Click
		msubKeyHit(Asc("4"c))
	End Sub

	Private Sub Button5_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button5.Click
		msubKeyHit(Asc("5"c))
	End Sub

	Private Sub Button6_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button6.Click
		msubKeyHit(Asc("6"c))
	End Sub

	Private Sub Button7_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button7.Click
		msubKeyHit(Asc("7"c))
	End Sub

	Private Sub Button8_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button8.Click
		msubKeyHit(Asc("8"c))
	End Sub

	Private Sub Button9_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button9.Click
		msubKeyHit(Asc("9"c))
	End Sub

	Private Sub Button10_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button10.Click
		msubKeyHit(Asc("0"c))
	End Sub

	Private Sub Button11_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button11.Click
		msubKeyHit(&HBD)
	End Sub

	Private Sub Button12_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button12.Click
		msubKeyHit(Asc("Q"c))
	End Sub

	Private Sub Button13_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button13.Click
		msubKeyHit(Asc("W"c))
	End Sub

	Private Sub Button14_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button14.Click
		msubKeyHit(Asc("E"c))
	End Sub

	Private Sub Button15_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button15.Click
		msubKeyHit(Asc("R"c))
	End Sub

	Private Sub Button16_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button16.Click
		msubKeyHit(Asc("T"c))
	End Sub

	Private Sub Button17_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button17.Click
		msubKeyHit(Asc("Y"c))
	End Sub

	Private Sub Button18_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button18.Click
		msubKeyHit(Asc("U"c))
	End Sub

	Private Sub Button19_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button19.Click
		msubKeyHit(Asc("I"c))
	End Sub

	Private Sub Button20_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button20.Click
		msubKeyHit(Asc("O"c))
	End Sub

	Private Sub Button21_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button21.Click
		msubKeyHit(Asc("P"c))
	End Sub

	Private Sub Button22_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button22.Click
		msubKeyHit(&HC0)
	End Sub

	Private Sub Button23_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button23.Click
		msubKeyHit(Asc("A"c))
	End Sub

	Private Sub Button24_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button24.Click
		msubKeyHit(Asc("S"c))
	End Sub

	Private Sub Button25_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button25.Click
		msubKeyHit(Asc("D"c))
	End Sub

	Private Sub Button26_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button26.Click
		msubKeyHit(Asc("F"c))
	End Sub

	Private Sub Button27_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button27.Click
		msubKeyHit(Asc("G"c))
	End Sub

	Private Sub Button28_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button28.Click
		msubKeyHit(Asc("H"c))
	End Sub

	Private Sub Button29_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button29.Click
		msubKeyHit(Asc("J"c))
	End Sub

	Private Sub Button30_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button30.Click
		msubKeyHit(Asc("K"c))
	End Sub

	Private Sub Button31_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button31.Click
		msubKeyHit(Asc("L"c))
	End Sub

	Private Sub Button32_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button32.Click
		msubKeyHit(&HBB)
	End Sub

	Private Sub Button33_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button33.Click
		msubKeyHit(&HBA)
	End Sub

	Private Sub Button34_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button34.Click
		msubKeyHit(Asc("Z"c))
	End Sub

	Private Sub Button35_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button35.Click
		msubKeyHit(Asc("X"c))
	End Sub

	Private Sub Button36_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button36.Click
		msubKeyHit(Asc("C"c))
	End Sub

	Private Sub Button37_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button37.Click
		msubKeyHit(Asc("V"c))
	End Sub

	Private Sub Button38_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button38.Click
		msubKeyHit(Asc("B"c))
	End Sub

	Private Sub Button39_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button39.Click
		msubKeyHit(Asc("N"c))
	End Sub

	Private Sub Button40_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button40.Click
		msubKeyHit(Asc("M"c))
	End Sub

	Private Sub Button41_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button41.Click
		msubKeyHit(&HBC)
	End Sub

	Private Sub Button42_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button42.Click
		msubKeyHit(&HBE)
	End Sub

	Private Sub Button43_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button43.Click
		msubKeyHit(&HBF)
	End Sub

	Private Sub Button44_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button44.Click
		msubKeyHit(&H15)
	End Sub

	Private Sub Button45_CheckedChanged(ByVal sender As Object, ByVal e As System.EventArgs) Handles Button45.CheckedChanged
		If mblnSemaphore Then Exit Sub
		Dim blnPrevPressed As Boolean = (Not Button45.Checked OrElse Button53.Checked)
		If Button45.Checked Then
			Button45.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT_PRESSED
		Else
			Button45.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT
		End If
		msubUpdateShift(blnPrevPressed)
	End Sub

	Private Sub Button46_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button46.Click
		msubKeyHit(&H24)
	End Sub

	Private Sub Button47_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button47.Click
		msubKeyHit(&H2E)
	End Sub

	Private Sub Button48_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button48.Click
		msubKeyHit(&H20)
	End Sub

	Private Sub Button49_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button49.Click
		msubKeyHit(&H28)
	End Sub

	Private Sub Button50_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button50.Click
		msubKeyHit(&H27)
	End Sub

	Private Sub Button51_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button51.Click
		msubKeyHit(&H13)
	End Sub

	Private Sub Button52_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button52.Click
		msubKeyHit(&HD)
	End Sub

	Private Sub Button53_CheckedChanged(ByVal sender As Object, ByVal e As System.EventArgs) Handles Button53.CheckedChanged
		If mblnSemaphore Then Exit Sub
		Dim blnPrevPressed As Boolean = (Button45.Checked OrElse Not Button53.Checked)
		If Button53.Checked Then
			Button53.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT_PRESSED
		Else
			Button53.Image = Global.mz80k_kbd.My.Resources.Resources.SHIFT
		End If
		msubUpdateShift(blnPrevPressed)
	End Sub

	Private Sub Button54_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button54.Click
		msubKeyHit(&HDE)
	End Sub

	Private Sub Button55_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button55.Click
		msubKeyHit(&H70)
	End Sub

	Private Sub Button56_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button56.Click
		msubKeyHit(&H6F)
	End Sub

	Private Sub Button57_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button57.Click
		msubKeyHit(&H6A)
	End Sub

	Private Sub Button58_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button58.Click
		msubKeyHit(&HDC)
	End Sub

	Private Sub Button59_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button59.Click
		msubKeyHit(&H71)
	End Sub

	Private Sub Button60_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button60.Click
		msubKeyHit(&H67)
	End Sub

	Private Sub Button61_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button61.Click
		msubKeyHit(&H68)
	End Sub

	Private Sub Button62_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button62.Click
		msubKeyHit(&HDB)
	End Sub

	Private Sub Button63_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button63.Click
		msubKeyHit(&H72)
	End Sub

	Private Sub Button64_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button64.Click
		msubKeyHit(&H64)
	End Sub

	Private Sub Button65_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button65.Click
		msubKeyHit(&H65)
	End Sub

	Private Sub Button66_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button66.Click
		msubKeyHit(&HDD)
	End Sub

	Private Sub Button67_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button67.Click
		msubKeyHit(&H73)
	End Sub

	Private Sub Button68_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button68.Click
		msubKeyHit(&H61)
	End Sub

	Private Sub Button69_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button69.Click
		msubKeyHit(&H62)
	End Sub

	Private Sub Button70_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button70.Click
		msubKeyHit(&HE2)
	End Sub

	Private Sub Button71_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button71.Click
		msubKeyHit(&H74)
	End Sub

	Private Sub Button72_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button72.Click
		msubKeyHit(&H75)
	End Sub

	Private Sub Button73_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button73.Click
		msubKeyHit(&H76)
	End Sub

	Private Sub Button74_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button74.Click
		msubKeyHit(&H6D)
	End Sub

	Private Sub Button75_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button75.Click
		msubKeyHit(&H69)
	End Sub

	Private Sub Button76_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button76.Click
		msubKeyHit(&H66)
	End Sub

	Private Sub Button77_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button77.Click
		msubKeyHit(&H63)
	End Sub

	Private Sub Button78_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button78.Click
		msubKeyHit(&H77)
	End Sub
End Class
