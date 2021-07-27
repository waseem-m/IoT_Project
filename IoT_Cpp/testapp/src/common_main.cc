// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

#include "firebase/auth.h"
#include "firebase/auth/user.h"
#include "firebase/firestore.h"
#include "firebase/util.h"

// Thin OS abstraction layer.
#include "main.h"  // NOLINT


//------------------------------------------------------
/*
* Arduino Includes
*/
#include "SerialPort.h"
//------------------------------------------------------


//------------------------------------------------------
/*
* PianoHandler Includes
*/
#include "PianoHandler.h"
//------------------------------------------------------


// Change to true to be able to play without a lesson
#define TRAIN_MODE false


//------------------------------------------------------
/*
* My 'using'
*/
using firebase::App;
using firebase::auth::Auth;
using namespace firebase::firestore;
using std::ifstream;
using std::ios;
using std::cout;
using std::endl;
using std::string;
//------------------------------------------------------

//------------------------------------------------------
/*
* My globals
*/
App* app;
Auth* auth = nullptr;
const char* email = "sarah1@gmail.com";
const char* password = "123456";
const char* system_uid = "0hYJMiVcjZuSJHH6fs8V";
Firestore* firestore = nullptr;
std::string student_email;
char output[MAX_DATA_LENGTH];
char* port = "\\\\.\\COM3";
char incoming[MAX_DATA_LENGTH];
SerialPort arduino(port);
PianoHandler* mPianoHandler;
CollectionReference lessons_ref;
DocumentReference system_ref;
CollectionReference students_ref;
//------------------------------------------------------

//------------------------------------------------------
/*
* FireStore constants
*/
const int kTimeoutMs = 5000;
const int kSleepMs = 100;
//------------------------------------------------------



bool OpenCollection(CollectionReference& coll, const char* collection_name);

// Waits for a Future to be completed and returns whether the future has
// completed successfully. If the Future returns an error, it will be logged.
bool Await(const firebase::FutureBase& future, const char* name) {
	int remaining_timeout = kTimeoutMs;
	while (future.status() == firebase::kFutureStatusPending &&
		remaining_timeout > 0) {
		remaining_timeout -= kSleepMs;
		ProcessEvents(kSleepMs);
	}

	if (future.status() != firebase::kFutureStatusComplete) {
		LogMessage("ERROR: %s returned an invalid result.", name);
		return false;
	}
	else if (future.error() != 0) {
		LogMessage("ERROR: %s returned error %d: %s", name, future.error(),
			future.error_message());
		return false;
	}
	return true;
}

class Countable {
public:
	int event_count() const { return event_count_; }

protected:
	int event_count_ = 0;
};

template <typename T>
class TestEventListener : public Countable,
	public EventListener<T> {
public:
	explicit TestEventListener(std::string name) : name_(std::move(name)) {}

	void OnEvent(const T& value, const firebase::firestore::Error error_code,
		const std::string& error_message) override {
		event_count_++;
		DocumentSnapshot snapshot = (DocumentSnapshot)value;
		if (error_code != kErrorOk) {
			LogMessage("ERROR: EventListener %s got %d (%s).", name_.c_str(),
				error_code, error_message.c_str());
		}
		else if (!snapshot.exists()) {
			LogMessage("Current data = NULL");
		}
		else {
			LogMessage("Document snapshot listener read:");
			for (const auto& kv : snapshot.GetData()) {
				if (kv.second.type() ==
					FieldValue::Type::kString) {
					LogMessage("\tkey is %s, value is %s", kv.first.c_str(),
						kv.second.string_value().c_str());
				}
				else if (kv.second.type() ==
					FieldValue::Type::kInteger) {
					LogMessage("\tkey is %s, value is %ld", kv.first.c_str(),
						kv.second.integer_value());
				}
				else {
					// Log unexpected type for debugging.
					LogMessage("\tkey is %s, value is neither string nor integer",
						kv.first.c_str());
				}
			}
		}
	}

	// Hides the STLPort-related quirk that `AddSnapshotListener` has different
	// signatures depending on whether `std::function` is available.
	template <typename U>
	ListenerRegistration AttachTo(U* ref) {
#if !defined(STLPORT)
		return ref->AddSnapshotListener(
			[this](const T& result, Error error_code,
				const std::string& error_message) {
					OnEvent(result, error_code, error_message);
			});
#else
		return ref->AddSnapshotListener(this);
#endif
	}

private:
	std::string name_;
};

void Await(const Countable& listener, const char* name) {
	int remaining_timeout = kTimeoutMs;
	while (listener.event_count() && remaining_timeout > 0) {
		remaining_timeout -= kSleepMs;
		ProcessEvents(kSleepMs);
	}
	if (remaining_timeout <= 0) {
		LogMessage("ERROR: %s listener timed out.", name);
	}
}

// Gets note in range [0,12]
void SendNoteToArduino(int note) {
	if (arduino.isConnected()) {
		LogMessage("Info: Sending note to arduino");
		output[0] = 'A' + note;
		output[1] = '\n';
		arduino.writeSerialPort(output, 2);
	}
	else {
		LogMessage("Error: Failed to send note to arduino - not connected");
	}
}

// Gets one of the strings:
// "all wrong\n","not matched\n","matched\n","waiting\n"
void SendStringToArduino(std::string arduino_string) {
	if (arduino.isConnected()) {
		LogMessage("Info: Sending string to arduino");
		for (int i = 0; i < arduino_string.size(); i++) {
			output[i] = arduino_string[i];
		}
		arduino.writeSerialPort(output, arduino_string.size());
	}
	else {
		LogMessage("Error: Failed to send string to arduino - not connected");
	}
}


void PlaySongLightsAndSounds(std::vector<FieldValue>& song_arr) {

	for (auto note_fieldval : song_arr) {
		std::string note_string = note_fieldval.string_value();
		int note_num = note_string[0] - 'a';
		SendNoteToArduino(note_num);
		mPianoHandler->PlayNote(note_num);
		Sleep(500);
	}
}



// function to find out the minimum penalty
void getMinimumPenalty(std::string x, std::string y, int pxy, int pgap)
{
	int i, j; // intialising variables

	int m = x.length(); // length of gene1
	int n = y.length(); // length of gene2

	std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

	for (size_t i = 0; i < m; ++i)
	{
		for (size_t j = 0; j < n; ++j)
		{
			dp[i][j] = 0;
		}
	}

	// intialising the table
	for (i = 0; i <= n; i++)
	{
		dp[i][0] = i * pgap;
	}
	for (i = 0; i <= m; i++)
	{
		dp[0][i] = i * pgap;
	}

	// calcuting the minimum penalty
	for (i = 1; i <= m; i++)
	{
		for (j = 1; j <= n; j++)
		{
			if (x[i - 1] == y[j - 1])
			{
				dp[i][j] = dp[i - 1][j - 1];
			}
			else
			{
				dp[i][j] = min(min(dp[i - 1][j - 1] + pxy,
					dp[i - 1][j] + pgap),
					dp[i][j - 1] + pgap);
			}
		}
	}

	// Reconstructing the solution
	int l = n + m; // maximum possible length

	i = m; j = n;

	int xpos = l;
	int ypos = l;

	// Final answers for the respective strings
	std::vector<int> xans(l + 1);
	std::vector<int> yans(l + 1);

	while (!(i == 0 || j == 0))
	{
		if (x[i - 1] == y[j - 1])
		{
			xans[xpos--] = (int)x[i - 1];
			yans[ypos--] = (int)y[j - 1];
			i--; j--;
		}
		else if (dp[i - 1][j - 1] + pxy == dp[i][j])
		{
			xans[xpos--] = (int)x[i - 1];
			yans[ypos--] = (int)y[j - 1];
			i--; j--;
		}
		else if (dp[i - 1][j] + pgap == dp[i][j])
		{
			xans[xpos--] = (int)x[i - 1];
			yans[ypos--] = (int)'_';
			i--;
		}
		else if (dp[i][j - 1] + pgap == dp[i][j])
		{
			xans[xpos--] = (int)'_';
			yans[ypos--] = (int)y[j - 1];
			j--;
		}
	}
	while (xpos > 0)
	{
		if (i > 0) xans[xpos--] = (int)x[--i];
		else xans[xpos--] = (int)'_';
	}
	while (ypos > 0)
	{
		if (j > 0) yans[ypos--] = (int)y[--j];
		else yans[ypos--] = (int)'_';
	}

	// Since we have assumed the answer to be n+m long,
	// we need to remove the extra gaps in the starting
	// id represents the index from which the arrays
	// xans, yans are useful
	int id = 1;
	for (i = l; i >= 1; i--)
	{
		if ((char)yans[i] == '_' && (char)xans[i] == '_')
		{
			id = i + 1;
			break;
		}
	}

	// Printing the final answer
	std::cout << "Minimum Penalty in aligning the genes = ";
	std::cout << dp[m][n] << "\n";
	std::cout << "The aligned genes are :\n";
	for (i = id; i <= l; i++)
	{
		std::cout << (char)xans[i];
	}
	std::cout << "\n";
	for (i = id; i <= l; i++)
	{
		std::cout << (char)yans[i];
	}
	std::cout << "\n";

	return;
}

void addPoints(size_t size, size_t correct) {
	if (correct == size) {
		correct *= 2;
	}
	size_t prev_points = 0;
	//read from firestore



	firebase::Future<QuerySnapshot> query_future =
		students_ref.WhereEqualTo("studentEmail", FieldValue::String(student_email)).Get();
	Await(query_future, "Query student");
	const QuerySnapshot* query_result = query_future.result();
	if (query_result) {
		//todo: handle failure to snapshot (possibly loop).
		const QuerySnapshot query_snapshot = *query_result;
		//should be only one document
		for (DocumentSnapshot doc_snap : query_snapshot.documents()) {
			FieldValue student_points = doc_snap.Get("points");
			if (!student_points.is_valid() || !student_points.is_integer()) {
				continue;
			}
			prev_points = student_points.integer_value();
			Await(doc_snap.reference().Update(
				MapFieldValue{
				{"points", FieldValue::Integer(prev_points + correct)}
				}), "Updated points of student");
		}
	}


}

/**
 * @brief Reads from RealSense and returns compare result.

 * @param[in] doc_snap		- snapshot of the lesson's document.
 * @param[in] teacher_notes - the notes (from firestore) that get compared.
 *
 * @return returns 0 if matched, -1 if all wrong, number of correct if not matched.
 *
 */
int RealSense_Compare(DocumentSnapshot& doc_snap, std::vector<FieldValue> teacher_notes) {

	std::vector<FieldValue> rs_notes_vector;
	size_t correct = 0;
	size_t song_length = teacher_notes.size();

	// for log
	std::string rs_strings;
	std::string teacher_strings;

	for (int i = 0; i < song_length; i++) {
		// teacher_note_str = ['a', 'm']
		std::string teacher_note_str = teacher_notes.at(i).string_value();
		// teacher_note = [0,12]
		int teacher_note = teacher_note_str[0] - 'a';
		
		// rs_note = [0,12]
		int rs_note = -1;
		while (rs_note == -1) {
			rs_note = mPianoHandler->GetNextKey();
		}
		rs_note = mPianoHandler->ConvertKeyToRange(rs_note);

		// After reading note, if i == 0, update isBegin
		if (i == 0) {
			Await(doc_snap.reference().Update(
				MapFieldValue{
				{"isBegin", FieldValue::Boolean(true)}
				}), "Update isBegin to true");
		}

		// Send to arduino for LED, and play note
		SendNoteToArduino(rs_note);
		mPianoHandler->PlayNote(rs_note);

		char rs_note_char = 'a' + rs_note;

		std::string rs_note_str(1,rs_note_char);
		rs_notes_vector.push_back(FieldValue::String(rs_note_str));
		Await(doc_snap.reference().Update(
			MapFieldValue{
			{"studentKeys", FieldValue::Array(rs_notes_vector)}
			}), "Add studentKeys to document");
		
		// for log
		rs_strings += rs_note_str;
		teacher_strings += teacher_note_str;

		if (teacher_note != rs_note) {
			break;
		}
		correct++;
	}

	LogMessage("teacher notes = '%s'", teacher_strings.c_str());
	LogMessage("realsense notes = '%s'", rs_strings.c_str());
	//todo: minimum penalty, how does it work with real-time?
	//getMinimumPenalty(teacher_strings, rs_strings, 1, 1);
	//return teacher_strings.compare(rs_strings);

	// returns -1 if all wrong
	if (correct == 0) {
		return -1;
	}

	// if not all wrong, need to update points
	addPoints(song_length, correct);

	// returns number of correct if not matched
	if (correct != song_length) {
		return correct;
	}

	// returns 0 if matched
	return 0;

}

//void FillTeachersArray(DocumentReference& doc, bool is_fill) {
//	if (!is_fill) {
//		return;
//	}
//	std::vector<FieldValue> my_list;
//	my_list.push_back(FieldValue::String("my_test_teacher1"));
//	my_list.push_back(FieldValue::String("my_test_teacher2"));
//	my_list.push_back(FieldValue::String("my_test_teacher3"));
//	Await(doc.Update(MapFieldValue{
//			  {t_arr_name, FieldValue::Array(my_list)} }
//	), ("Fill TeachersArray of student '" + std::string(student_uid) + "'").c_str());
//
//}

/**
 * @brief Gets snapshot of document in 'doc'.

 * @param[out] snapshot - pointer to DocumentSnapshot, where the result will be stored.
 * @param[in] doc - DocumentReference to the document.
 * @param[in] name - the name of the document to be opened.
 *
 * @return returns true if successful, otherwise returns false.
 *
 */
bool GetSnapshot(const DocumentSnapshot*& snapshot, DocumentReference& doc, std::string doc_name) {
	auto doc_future = doc.Get();
	std::string await_msg = doc_name + ".Get";
	if (Await(doc_future, await_msg.c_str())) {
		snapshot = doc_future.result();
		if (snapshot == nullptr) {
			LogMessage("ERROR: failed to read '%s' document.", doc_name);
			return false;
		}
		return true;
	}
}

/**
 * @brief Opens collection.

 * @param[out] coll - CollectionReference where the result will be stored.
 * @param[in] name - The name of the collection to be opened.
 *
 */
bool OpenCollection(CollectionReference& coll, const char* collection_name) {
	LogMessage("Opening '%s' collection...", collection_name);
	coll = firestore->Collection(collection_name);
	if (coll.firestore() != firestore) {
		LogMessage("ERROR: failed to get Firestore from document.");
		return false;
	}

	if (!coll.path()._Equal(collection_name)) {
		LogMessage("ERROR: failed to get path string from document.");
		return false;
	}
	LogMessage("Opened '%s' document.", collection_name);
	return true;
}

/**
 * @brief Opens document 'document_name' in collection('collection_name').

 * @param[out] doc - DocumentReference where the result will be stored.
 * @param[in] document_name - The name of the document to be opened.
 * @param[in] collection_name - The name of the collection containintg the document.
 *
 */
bool OpenDocument(DocumentReference& doc, const char* document_name, const char* collection_name) {
	std::string path = collection_name + std::string("/") + std::string(document_name);
	LogMessage("Opening '%s' document...", path.c_str());
	doc = firestore->Document(path);
	if (doc.firestore() != firestore) {
		LogMessage("ERROR: failed to get Firestore from document.");
		return false;
	}

	if (doc.path() != path) {
		LogMessage("ERROR: failed to get path string from document.");
		return false;
	}
	LogMessage("Opened '%s' document.", path.c_str());
	return true;
}


/**
 * @brief Initializer function.
 *
 * Initializes App, Firebase Auth (with email & password as params), and Firestore.
 *
 * @return returns 0 if successful, otherwise returns non-zero.
 */
int InitializeAndSignIn() {

	app = App::Create();

	LogMessage("Initialized Firebase App.");

	LogMessage("Initializing Firebase Auth...");
	firebase::InitResult result;
	auth = Auth::GetAuth(app, &result);
	if (result != firebase::kInitResultSuccess) {
		LogMessage("Failed to initialize Firebase Auth, error: %d",
			static_cast<int>(result));
		return -1;
	}
	LogMessage("Initialized Firebase Auth.");

	// Auth caches the previously signed-in user, which can be annoying when
	// trying to test for sign-in failures.
	auth->SignOut();

	LogMessage("Signing in with email='%s', password='%s'...", email, password);
	auto login_future = auth->SignInWithEmailAndPassword(email, password);
	Await(login_future, "Auth sign-in");
	auto* login_result = login_future.result();
	if (login_result && *login_result) {
		const firebase::auth::User* user = *login_result;
		LogMessage("Signed in as %s user, uid: %s, email: %s.\n",
			user->is_anonymous() ? "an anonymous" : "a non-anonymous",
			user->uid().c_str(), user->email().c_str());
		//student_uid = user->uid();
		student_email = user->email();
	}
	else {
		LogMessage("ERROR: could not sign in");
		return -1;
	}

	// Note: Auth cannot be deleted while any of the futures issued by it are
	// still valid.
	login_future.Release();

	LogMessage("Initialize Firebase Firestore.");

	// Use ModuleInitializer to initialize Database, ensuring no dependencies are
	// missing.
	firestore = nullptr;
	void* initialize_targets[] = { &firestore };

	const firebase::ModuleInitializer::InitializerFn initializers[] = {
		[](App* app, void* data) {
		  LogMessage("Attempt to initialize Firebase Firestore.");
		  void** targets = reinterpret_cast<void**>(data);
		  firebase::InitResult result;
		  *reinterpret_cast<Firestore**>(targets[0]) =
			  Firestore::GetInstance(app, &result);
		  return result;
		} };

	firebase::ModuleInitializer initializer;
	initializer.Initialize(app, initialize_targets, initializers,
		sizeof(initializers) / sizeof(initializers[0]));

	Await(initializer.InitializeLastResult(), "Initialize");

	if (initializer.InitializeLastResult().error() != 0) {
		LogMessage("Failed to initialize Firebase libraries: %s",
			initializer.InitializeLastResult().error_message());
		return -1;
	}
	LogMessage("Successfully initialized Firebase Firestore.");

	//firestore->set_log_level(firebase::kLogLevelDebug);
	firestore->set_log_level(firebase::kLogLevelWarning);
	//firestore->set_log_level(firebase::kLogLevelAssert);

	if (firestore->app() != app) {
		LogMessage("ERROR: failed to get App the Firestore was created with.");
	}

	Settings settings = firestore->settings();
	firestore->set_settings(settings);
	LogMessage("Successfully set Firestore settings.");

	return 0;
}

void run_cam() {
	int rs_note = -1;
	while (rs_note == -1) {
		rs_note = mPianoHandler->GetNextKey();
	}
	rs_note = mPianoHandler->ConvertKeyToRange(rs_note);

	// Send to arduino for LED, and play note
	//SendNoteToArduino(rs_note);
	mPianoHandler->PlayNote(rs_note);
}

extern "C" int common_main(int argc, const char* argv[]) try {

	int init_res = InitializeAndSignIn();
	if (init_res != 0) {
		return init_res;
	}

	// Open "Students" collection for updating points.
	if (!OpenCollection(students_ref, "Students")) {
		LogMessage("Error: Failed to open Students collection");
		return -1;
	}

	// Open "Lessons" collection for reading lessons & updating keys.
	if (!OpenCollection(lessons_ref, "Lessons")) {
		LogMessage("Error: Failed to open Lessons collection");
		return -1;
	}

	// Open "System/{system_uid}" document for checking if sensors or pc.
	if (!OpenDocument(system_ref, system_uid, "System")) {
		LogMessage("Error: Failed to open System/[system_uid] document");
		return -1;
	}
	if (!system_ref.is_valid()) {
		LogMessage("Error: System/[system_uid] document is invalid");
		return -1;
	}

	//Initialize PianoHandler.
	mPianoHandler = &PianoHandler::GetInstance();

	//Configure Piano
	while (mPianoHandler->ConfigurePiano() != 0) {
	}

#if TRAIN_MODE == true
	mPianoHandler->OpenKeysWindow();
	while (1) {
		run_cam();
	}
	mPianoHandler->CloseKeysWindow();
#endif

	std::cout << "Press Ctrl+C to quit." << std::endl;
	while (!ProcessEvents(1000)) {

		//---------------------------------------------
		// Wait until System Sensors==false
		bool isSensor = true;

		while (isSensor) {

			const DocumentSnapshot* system_snapshot_pointer;
			if (!GetSnapshot(system_snapshot_pointer, system_ref, "System/{system_uid}")) {
				break;
			}

			const DocumentSnapshot system_snapshot = *system_snapshot_pointer;
			FieldValue sensors_fieldvalue = system_snapshot.Get("Sensors");
			if (sensors_fieldvalue.is_valid() == false) {
				LogMessage("Error: Failed to read a valid 'Sensors' field");
				break;
			}
			isSensor = sensors_fieldvalue.boolean_value();
			LogMessage("Info: Read Sensors=%s", ((isSensor) ? "true" : "false"));
		}
		//if sensor is true here, means that a break occured, continue to try again.
		if (isSensor == true) {
			continue;
		}
		//---------------------------------------------

		// Send "waiting\n" to arduino, until new lesson is found
		SendStringToArduino("waiting pc\n");

		//---------------------------------------------
		// Find a valid lesson
		LogMessage("Getting a lesson...");

		firebase::Future<QuerySnapshot> query_future =
			lessons_ref.WhereEqualTo("studentEmail", FieldValue::String(student_email))
			.WhereEqualTo("isBegin", FieldValue::Boolean(false))
			.WhereEqualTo("isConfirmed", FieldValue::Boolean(true))
			.OrderBy("time", Query::Direction::kDescending)
			.Limit(1)
			.Get();

		bool await_lessons = Await(query_future, "Query Lessons");
		if (!await_lessons) {
			LogMessage("ERROR: Lessons await failed");
			continue;
		}

		const QuerySnapshot* query_result = query_future.result();
		if (!query_result) {
			LogMessage("ERROR: Lessons query returned NULL");
			continue;
		}
		if (query_result->empty()) {
			LogMessage("Info: query result is empty (no lessons available)");
			continue;
		}
		//---------------------------------------------


		const QuerySnapshot query_snapshot = *query_result;
		// The lesson we got from query. Should be just one document.
		for (DocumentSnapshot doc_snap : query_snapshot.documents()) {

			//std::string teacher_uid = doc_snap.Get("teacherUid").string_value();
			//LogMessage("teacherUid = %s", teacher_uid.c_str());

			//FieldValue student_notes = doc_snap.Get("studentKeys");
			//if (!student_notes.is_valid() || !student_notes.is_array()
			//	|| student_notes.array_value().size() != 0) {
			//	continue;
			//}

			std::vector<FieldValue> song_arr = doc_snap.Get("teacherKeys").array_value();

			PlaySongLightsAndSounds(song_arr);

			//std::vector<FieldValue> rs_arr;
			mPianoHandler->OpenKeysWindow();
			int compare_res = RealSense_Compare(doc_snap, song_arr); //updates isBegin
			mPianoHandler->CloseKeysWindow();
			LogMessage("Compare result = %d", compare_res);
			
			// Turn on error lights and sounds of result
			switch (compare_res) {
			case -1:
				SendStringToArduino("all wrong\n");
				break;
			case 0:
				SendStringToArduino("matched\n");
				break;
			default:
				SendStringToArduino("not matched\n");
				break;
			}
		}

	}

	// Keeping in comments for reference
	/*LogMessage("Testing collections.");
	CollectionReference collection =
		firestore->Collection("Tones");
	if (collection.id() != "Tones") {
		LogMessage("ERROR: failed to get collection id.");
	}
	if (collection.Document("tone").path() != "Tones/tone") {
		LogMessage("ERROR: failed to get path of a nested document.");
	}
	LogMessage("Tested collections.");

	LogMessage("Testing Set().");
	DocumentReference new_document =
		firestore->Document("test_read_coll/my_cpp_doc");
	if (new_document.firestore() != firestore) {
		LogMessage("ERROR: failed to get Firestore from document.");
	}
	std::vector<FieldValue> my_list;
	my_list.push_back(FieldValue::String("aaa"));
	my_list.push_back(FieldValue::String("bbb"));
	my_list.push_back(FieldValue::String("ccc"));
	Await(new_document.Set(MapFieldValue{
			  {"arr", FieldValue::Array(my_list)},
			  {"int", FieldValue::Integer(111)} }),
			  "new_document.Set");
	LogMessage("Tested Set().");

	LogMessage("Testing Update().");
	Await(document.Update(MapFieldValue{
			  {"int", FieldValue::Integer(321)} }),
			  "document.Update");

	LogMessage("Testing Delete().");
	Await(document.Delete(), "document.Delete");
	LogMessage("Tested document operations.");

	LogMessage("Testing teachers_doc snapshot listener:");
	TestEventListener<DocumentSnapshot>
		document_event_listener{ "for teachers_doc" };
	ListenerRegistration registration =
		document_event_listener.AttachTo(&teachers_doc);
	Await(document_event_listener, "teachers_doc.AddSnapshotListener");
	LogMessage("Successfully added teachers_doc snapshot listener.");
	registration.Remove();
	LogMessage("Successfully removed document snapshot listener.");

	LogMessage("Testing batch write.");
	WriteBatch batch = firestore->batch();
	batch.Set(collection.Document("one"),
		MapFieldValue{
			{"str", FieldValue::String("foo")} });
	batch.Set(collection.Document("two"),
		MapFieldValue{
			{"int", FieldValue::Integer(123)} });
	Await(batch.Commit(), "batch.Commit");
	LogMessage("Tested batch write.");

	LogMessage("Testing transaction.");
	Await(
		firestore->RunTransaction(
			[collection](Transaction& transaction,
				std::string&) -> Error {
					transaction.Update(
						collection.Document("one"),
						MapFieldValue{
							{"int", FieldValue::Integer(123)} });
					transaction.Delete(collection.Document("two"));
					transaction.Set(
						collection.Document("three"),
						MapFieldValue{
							{"int", FieldValue::Integer(321)} });
					return kErrorOk;
			}),
		"firestore.RunTransaction");
	LogMessage("Tested transaction.");

	LogMessage("Testing query.");
	Query query =
		collection
		.WhereGreaterThan("int",
			FieldValue::Boolean(true))
		.Limit(3);
	auto query_future = query.Get();
	if (Await(query_future, "query.Get")) {
		const QuerySnapshot* snapshot = query_future.result();
		if (snapshot == nullptr) {
			LogMessage("ERROR: failed to fetch query result.");
		}
		else {
			for (const auto& doc : snapshot->documents()) {
				if (doc.id() == "one" || doc.id() == "three") {
					LogMessage("doc %s is %ld", doc.id().c_str(),
						doc.Get("int").integer_value());
				}
				else {
					LogMessage("ERROR: unexpected document %s.", doc.id().c_str());
				}
			}
		}
	}
	else {
		LogMessage("ERROR: failed to fetch query result.");
	}
	LogMessage("Tested query.");*/


	LogMessage("Shutdown the Firestore library.");
	delete firestore;
	firestore = nullptr;

	LogMessage("Shutdown Auth.");
	delete auth;
	LogMessage("Shutdown Firebase App.");
	delete app;


	LogMessage("App ran succesfully.");

	return 0;
}
catch (const rs2::error& e)
{
	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}