common source code projectにおける修正点

- fmtimer内で、Timer Aの値が負にならないようにシフト量を調整
- ReadIRQ()を追加
- リズム音源のファイルパスの型を*_TCHARに変更
- 各クラスにSaveState(),LoadState()を追加
- types.hの替りに、../../common.hをincludeするように変更
- FileIOクラスの替りに、FILEIOクラスを使用するように変更
- 各ボリュームを左右別に設定できるように変更

