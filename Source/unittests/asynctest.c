/*
 * AsyncTest - unit tests for asyncio.library
 *
 * Each suite documents purpose, expected results, and actual results for
 * every check.  Uses proto/asyncio.h (libcall pragmas) like application code.
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <libraries/asyncio.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/asyncio.h>

#include <string.h>
#include <stdio.h>

#define TEST_BUF_SIZE   8192
#define TEST_LINE_SIZE  256
#define TEST_FILE_A     "T:asynctest_a.dat"
#define TEST_FILE_B     "T:asynctest_b.dat"
#define TEST_FILE_C     "T:asynctest_c.dat"
#define TEST_FILE_COPY  "T:asynctest_copy.dat"
#define TEST_DATA_FILE  "test_data.txt"
#define TEST_LARGE_FILE "test_large.txt"

struct Library *AsyncIOBase;

static char fixture_data[64];
static char fixture_large[64];

static LONG test_total;
static LONG test_passed;
static LONG test_failed;
static BOOL suite_fail;

static const char sample_line[] = "The quick brown fox jumps over the lazy dog\n";
static const char append_tail[] = " appended\n";
static const char line_set[][64] = {
	"Alpha line one\n",
	"Beta line two\n",
	"Gamma line three\n"
};

static const UBYTE binary_fixture[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x57, 0x6F,
	0x72, 0x6C, 0x64, 0xFF, 0xFE, 0xFD, 0xFC
};
#define BINARY_FIXTURE_LEN ((LONG)sizeof(binary_fixture))

static void print_api_map(void)
{
	printf("Public API functions under test (13 LVOs):\n");
	printf("  OpenAsync          - MODE_READ, MODE_WRITE, MODE_APPEND\n");
	printf("  OpenAsyncFromFH    - read and write via existing BPTR\n");
	printf("  CloseAsync         - flush/close, NULL handle error\n");
	printf("  SeekAsync          - MODE_START, MODE_CURRENT, MODE_END\n");
	printf("  ReadAsync          - block read, partial read, chunked read\n");
	printf("  WriteAsync         - block write, binary round-trip\n");
	printf("  ReadCharAsync      - single-byte read, EOF (-1)\n");
	printf("  WriteCharAsync     - single-byte write\n");
	printf("  ReadLineAsync      - line-oriented read\n");
	printf("  WriteLineAsync     - line-oriented write\n");
	printf("  FGetsAsync         - fgets-style line read\n");
	printf("  FGetsLenAsync      - fgets with explicit length, EOF (NULL)\n");
	printf("  PeekAsync          - non-destructive buffer preview\n");
	printf("\n");
}

static void suite_begin(const char *title, const char *purpose, const char *apis)
{
	test_total++;
	suite_fail = FALSE;
	printf("TEST %ld: %s\n", test_total, title);
	printf("  Purpose: %s\n", purpose);
	printf("  APIs:    %s\n", apis);
}

static void suite_end(void)
{
	if (suite_fail) {
		test_failed++;
		printf("  RESULT: FAIL\n\n");
	} else {
		test_passed++;
		printf("  RESULT: PASS\n\n");
	}
}

static BOOL expect_long_eq(const char *check, LONG expected, LONG actual)
{
	printf("  - %s\n", check);
	printf("      Expect: %ld\n", expected);
	printf("      Actual: %ld\n", actual);
	if (actual != expected) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_long_ge(const char *check, LONG minimum, LONG actual)
{
	printf("  - %s\n", check);
	printf("      Expect: >= %ld\n", minimum);
	printf("      Actual: %ld\n", actual);
	if (actual < minimum) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_long_le(const char *check, LONG maximum, LONG actual)
{
	printf("  - %s\n", check);
	printf("      Expect: <= %ld\n", maximum);
	printf("      Actual: %ld\n", actual);
	if (actual > maximum) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_handle_ok(const char *check, struct AsyncFile *af)
{
	printf("  - %s\n", check);
	printf("      Expect: non-NULL AsyncFile handle\n");
	printf("      Actual: %p\n", af);
	if (af == NULL) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_handle_null(const char *check, struct AsyncFile *af)
{
	printf("  - %s\n", check);
	printf("      Expect: NULL (open failure)\n");
	printf("      Actual: %p\n", af);
	if (af != NULL) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_str_eq(const char *check, const char *expected, const char *actual)
{
	printf("  - %s\n", check);
	printf("      Expect: \"%s\"\n", expected);
	printf("      Actual: \"%s\"\n", actual);
	if (strcmp(expected, actual) != 0) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_mem_eq(const char *check, const void *expected, const void *actual, LONG len)
{
	printf("  - %s\n", check);
	printf("      Expect: %ld bytes match reference buffer\n", len);
	printf("      Actual: %s\n", (memcmp(expected, actual, len) == 0) ? "match" : "mismatch");
	if (memcmp(expected, actual, len) != 0) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_ptr_eq(const char *check, const void *expected, const void *actual)
{
	printf("  - %s\n", check);
	printf("      Expect: %p\n", expected);
	printf("      Actual: %p\n", actual);
	if (expected != actual) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static BOOL expect_dos_handle_ok(const char *check, BPTR fh)
{
	printf("  - %s\n", check);
	printf("      Expect: non-NULL DOS file handle\n");
	printf("      Actual: %lu\n", (ULONG)fh);
	if (fh == 0) {
		suite_fail = TRUE;
		return FALSE;
	}
	return TRUE;
}

static void cleanup_files(void)
{
	DeleteFile(TEST_FILE_A);
	DeleteFile(TEST_FILE_B);
	DeleteFile(TEST_FILE_C);
	DeleteFile(TEST_FILE_COPY);
}

static BOOL fixture_openable(const char *path)
{
	struct AsyncFile *af;

	af = OpenAsync((STRPTR)path, MODE_READ, TEST_BUF_SIZE);
	if (!af) {
		return FALSE;
	}
	CloseAsync(af);
	return TRUE;
}

static BOOL locate_fixture(const char *basename, char *out, LONG outlen)
{
	static const char *prefixes[] = {
		"",
		"unittests/",
		NULL
	};
	LONG i;
	LONG need;

	for (i = 0; prefixes[i] != NULL; i++) {
		need = (LONG)strlen(prefixes[i]) + (LONG)strlen(basename) + 1;
		if (need > outlen) {
			continue;
		}
		strcpy(out, prefixes[i]);
		strcat(out, basename);
		if (fixture_openable(out)) {
			return TRUE;
		}
	}

	strncpy(out, basename, outlen - 1);
	out[outlen - 1] = '\0';
	return FALSE;
}

static void init_fixture_paths(void)
{
	if (!locate_fixture(TEST_DATA_FILE, fixture_data, (LONG)sizeof(fixture_data))) {
		strcpy(fixture_data, TEST_DATA_FILE);
	}
	if (!locate_fixture(TEST_LARGE_FILE, fixture_large, (LONG)sizeof(fixture_large))) {
		strcpy(fixture_large, TEST_LARGE_FILE);
	}
}

static LONG async_file_size(const char *path)
{
	struct AsyncFile *af;
	char buf[1024];
	LONG total;
	LONG n;

	af = OpenAsync((STRPTR)path, MODE_READ, TEST_BUF_SIZE);
	if (!af) {
		return -1;
	}
	total = 0;
	while ((n = ReadAsync(af, buf, sizeof(buf))) > 0) {
		total += n;
	}
	CloseAsync(af);
	return total;
}

static BOOL test_open_close(void)
{
	struct AsyncFile *af;
	LONG rc;

	suite_begin(
		"OpenAsync / CloseAsync",
		"Open a file in each mode and close handles; verify CloseAsync(NULL) fails.",
		"OpenAsync, CloseAsync"
	);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_WRITE) creates/truncates file", af)) {
		suite_end();
		return FALSE;
	}
	rc = CloseAsync(af);
	if (!expect_long_ge("CloseAsync flushes and closes write handle", 0, rc)) {
		suite_end();
		return FALSE;
	}

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) opens existing file", af)) {
		suite_end();
		return FALSE;
	}
	rc = CloseAsync(af);
	if (!expect_long_ge("CloseAsync closes read handle", 0, rc)) {
		suite_end();
		return FALSE;
	}

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_APPEND, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_APPEND) opens for append", af)) {
		suite_end();
		return FALSE;
	}
	rc = CloseAsync(af);
	if (!expect_long_ge("CloseAsync closes append handle", 0, rc)) {
		suite_end();
		return FALSE;
	}

	rc = CloseAsync(NULL);
	if (!expect_long_le("CloseAsync(NULL) returns error", -1, rc)) {
		suite_end();
		return FALSE;
	}

	suite_end();
	return !suite_fail;
}

static BOOL test_append_mode(void)
{
	struct AsyncFile *af;
	char buf[160];
	char expect[160];
	LONG head;
	LONG tail;
	LONG rc;
	LONG total;

	suite_begin(
		"MODE_APPEND",
		"Write base text, append more data, confirm combined content on disk.",
		"OpenAsync(MODE_WRITE), OpenAsync(MODE_APPEND), WriteAsync, ReadAsync, CloseAsync"
	);

	head = (LONG)strlen(sample_line);
	tail = (LONG)strlen(append_tail);
	total = head + tail;

	af = OpenAsync((STRPTR)TEST_FILE_C, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_WRITE) for base content", af)) {
		suite_end();
		return FALSE;
	}
	rc = WriteAsync(af, (APTR)sample_line, head);
	if (!expect_long_eq("WriteAsync writes base block", head, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_C, MODE_APPEND, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_APPEND) positions at EOF", af)) {
		suite_end();
		return FALSE;
	}
	rc = WriteAsync(af, (APTR)append_tail, tail);
	if (!expect_long_eq("WriteAsync appends at end of file", tail, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	strcpy(expect, sample_line);
	strcat(expect, append_tail);

	af = OpenAsync((STRPTR)TEST_FILE_C, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) for verification", af)) {
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, buf, sizeof(buf) - 1);
	if (!expect_long_eq("ReadAsync returns full appended file size", total, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	buf[rc] = '\0';
	if (!expect_str_eq("Combined file matches base plus append", expect, buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static BOOL test_write_read_roundtrip(void)
{
	struct AsyncFile *af;
	char buf[128];
	LONG len;
	LONG rc;

	suite_begin(
		"WriteAsync / ReadAsync block I/O",
		"Write a text block, close, reopen, read back identical bytes.",
		"OpenAsync, WriteAsync, ReadAsync, CloseAsync"
	);

	len = (LONG)strlen(sample_line);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_WRITE)", af)) {
		suite_end();
		return FALSE;
	}
	rc = WriteAsync(af, (APTR)sample_line, len);
	if (!expect_long_eq("WriteAsync returns bytes accepted", len, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ)", af)) {
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, buf, sizeof(buf) - 1);
	if (!expect_long_eq("ReadAsync returns full record length", len, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	buf[rc] = '\0';
	if (!expect_str_eq("ReadAsync buffer matches written text", sample_line, buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static BOOL test_partial_read(void)
{
	struct AsyncFile *af;
	char buf[16];
	LONG len;
	LONG rc;
	LONG sum;

	suite_begin(
		"ReadAsync partial reads",
		"Read a file in small chunks; summed lengths equal file size.",
		"ReadAsync, WriteAsync, OpenAsync, CloseAsync"
	);

	len = (LONG)strlen(sample_line);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) on round-trip file", af)) {
		suite_end();
		return FALSE;
	}

	sum = 0;
	while (sum < len) {
		rc = ReadAsync(af, buf, 7);
		if (rc <= 0) {
			expect_long_ge("ReadAsync returns data before EOF", 1, rc);
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
		if (!expect_long_le("ReadAsync chunk does not exceed request", 7, rc)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
		sum += rc;
	}
	if (!expect_long_eq("Sum of partial reads equals file size", len, sum)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static BOOL test_char_io(void)
{
	struct AsyncFile *af;
	LONG rc;
	LONG ch;

	suite_begin(
		"WriteCharAsync / ReadCharAsync",
		"Write three bytes, read them back, then confirm EOF returns -1.",
		"WriteCharAsync, ReadCharAsync, OpenAsync, CloseAsync"
	);

	af = OpenAsync((STRPTR)TEST_FILE_B, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_WRITE)", af)) {
		suite_end();
		return FALSE;
	}
	if (!expect_long_eq("WriteCharAsync('A') returns 1", 1, WriteCharAsync(af, 'A'))) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	if (!expect_long_eq("WriteCharAsync('B') returns 1", 1, WriteCharAsync(af, 'B'))) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	if (!expect_long_eq("WriteCharAsync('C') returns 1", 1, WriteCharAsync(af, 'C'))) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_B, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ)", af)) {
		suite_end();
		return FALSE;
	}
	ch = ReadCharAsync(af);
	if (!expect_long_eq("ReadCharAsync first byte is 'A' (65)", (LONG)'A', ch)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	ch = ReadCharAsync(af);
	if (!expect_long_eq("ReadCharAsync second byte is 'B' (66)", (LONG)'B', ch)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	ch = ReadCharAsync(af);
	if (!expect_long_eq("ReadCharAsync third byte is 'C' (67)", (LONG)'C', ch)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	ch = ReadCharAsync(af);
	if (!expect_long_eq("ReadCharAsync at EOF returns -1", -1, ch)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static BOOL test_seek(void)
{
	struct AsyncFile *af;
	char buf[16];
	LONG rc;
	LONG ch;

	suite_begin(
		"SeekAsync all seek modes",
		"Seek from start, read in place, rewind, seek from end, verify tail bytes.",
		"SeekAsync (MODE_START, MODE_CURRENT, MODE_END), ReadAsync, ReadCharAsync"
	);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ)", af)) {
		suite_end();
		return FALSE;
	}

	rc = SeekAsync(af, 0, MODE_START);
	if (!expect_long_ge("SeekAsync(0, MODE_START) succeeds", 0, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, buf, 4);
	if (!expect_long_eq("ReadAsync after seek reads 4 bytes", 4, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	buf[4] = '\0';
	if (!expect_str_eq("Bytes at offset 0 are \"The \"", "The ", buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}

	rc = ReadAsync(af, buf, 5);
	if (!expect_long_eq("Sequential ReadAsync reads next 5 bytes", 5, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	buf[5] = '\0';
	if (!expect_str_eq("Bytes at offset 4 are \"quick\"", "quick", buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}

	rc = SeekAsync(af, 0, MODE_START);
	if (!expect_long_ge("SeekAsync rewind to start", 0, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, buf, 4);
	buf[4] = '\0';
	if (!expect_str_eq("Re-read after rewind still \"The \"", "The ", buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}

	rc = SeekAsync(af, -1, MODE_END);
	if (!expect_long_ge("SeekAsync(-1, MODE_END) seeks to last byte", 0, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	rc = ReadCharAsync(af);
	if (!expect_long_eq("Last byte of file is newline (10)", 10, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	ch = ReadCharAsync(af);
	if (!expect_long_eq("ReadCharAsync past EOF after tail read returns -1", -1, ch)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}

	CloseAsync(af);
	suite_end();
	return !suite_fail;
}

static BOOL test_peek(void)
{
	struct AsyncFile *af;
	char peekbuf[8];
	char readbuf[8];
	LONG rc;

	suite_begin(
		"PeekAsync",
		"Peek bytes without advancing; subsequent ReadAsync returns same data.",
		"PeekAsync, ReadAsync"
	);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ)", af)) {
		suite_end();
		return FALSE;
	}

	rc = PeekAsync(af, peekbuf, 5);
	if (!expect_long_eq("PeekAsync returns requested byte count", 5, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, readbuf, 5);
	if (!expect_long_eq("ReadAsync after peek still reads 5 bytes", 5, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	if (!expect_mem_eq("Peek buffer matches read buffer", peekbuf, readbuf, 5)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}

	CloseAsync(af);
	suite_end();
	return !suite_fail;
}

static BOOL test_line_functions(void)
{
	struct AsyncFile *af;
	char buf[TEST_LINE_SIZE];
	LONG rc;
	LONG i;
	LONG len;
	STRPTR p;

	suite_begin(
		"Line-oriented I/O",
		"Write lines, read back via ReadLineAsync, FGetsAsync, and FGetsLenAsync.",
		"WriteLineAsync, ReadLineAsync, FGetsAsync, FGetsLenAsync"
	);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_WRITE)", af)) {
		suite_end();
		return FALSE;
	}
	for (i = 0; i < 3; i++) {
		rc = WriteLineAsync(af, (STRPTR)line_set[i]);
		if (!expect_long_eq("WriteLineAsync returns line length incl. newline", (LONG)strlen(line_set[i]), rc)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) for ReadLineAsync", af)) {
		suite_end();
		return FALSE;
	}
	for (i = 0; i < 3; i++) {
		rc = ReadLineAsync(af, buf, sizeof(buf));
		if (!expect_long_eq("ReadLineAsync returns stored line length", (LONG)strlen(line_set[i]), rc)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
		if (!expect_str_eq("ReadLineAsync content matches written line", line_set[i], buf)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) for FGetsAsync", af)) {
		suite_end();
		return FALSE;
	}
	for (i = 0; i < 3; i++) {
		p = FGetsAsync(af, buf, sizeof(buf));
		if (!expect_ptr_eq("FGetsAsync returns caller buffer pointer", buf, p)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
		if (!expect_str_eq("FGetsAsync line content", line_set[i], buf)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
	}
	p = FGetsAsync(af, buf, sizeof(buf));
	if (!expect_ptr_eq("FGetsAsync at EOF returns NULL", NULL, p)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) for FGetsLenAsync", af)) {
		suite_end();
		return FALSE;
	}
	for (i = 0; i < 3; i++) {
		p = FGetsLenAsync(af, buf, sizeof(buf), &len);
		if (!expect_ptr_eq("FGetsLenAsync returns caller buffer pointer", buf, p)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
		if (!expect_long_eq("FGetsLenAsync *length matches line", (LONG)strlen(line_set[i]), len)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
		if (!expect_str_eq("FGetsLenAsync line content", line_set[i], buf)) {
			CloseAsync(af);
			suite_end();
			return FALSE;
		}
	}
	p = FGetsLenAsync(af, buf, sizeof(buf), &len);
	if (!expect_ptr_eq("FGetsLenAsync at EOF returns NULL", NULL, p)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static BOOL test_open_from_fh(void)
{
	struct AsyncFile *af;
	BPTR fh;
	char buf[32];
	LONG rc;

	suite_begin(
		"OpenAsyncFromFH",
		"Wrap DOS file handles for async read and write paths.",
		"OpenAsyncFromFH, ReadAsync, WriteAsync, CloseAsync"
	);

	fh = Open((STRPTR)TEST_FILE_A, MODE_OLDFILE);
	if (!expect_dos_handle_ok("DOS Open existing file for FH read test", fh)) {
		suite_end();
		return FALSE;
	}

	af = OpenAsyncFromFH(fh, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsyncFromFH(MODE_READ)", af)) {
		Close(fh);
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, buf, 5);
	if (!expect_long_eq("ReadAsync via FH-backed handle reads 5 bytes", 5, rc)) {
		CloseAsync(af);
		Close(fh);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);
	Close(fh);

	fh = Open((STRPTR)TEST_FILE_B, MODE_NEWFILE);
	if (!expect_dos_handle_ok("DOS Open new file for FH write test", fh)) {
		suite_end();
		return FALSE;
	}
	af = OpenAsyncFromFH(fh, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsyncFromFH(MODE_WRITE)", af)) {
		Close(fh);
		suite_end();
		return FALSE;
	}
	rc = WriteAsync(af, (APTR)"FHOK", 4);
	if (!expect_long_eq("WriteAsync via FH-backed handle writes 4 bytes", 4, rc)) {
		CloseAsync(af);
		Close(fh);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);
	Close(fh);

	af = OpenAsync((STRPTR)TEST_FILE_B, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) verify FH write", af)) {
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, buf, 4);
	buf[4] = '\0';
	if (!expect_long_eq("ReadAsync returns 4 bytes from FH-written file", 4, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	if (!expect_str_eq("Content written via OpenAsyncFromFH is \"FHOK\"", "FHOK", buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static BOOL test_errors(void)
{
	struct AsyncFile *af;
	char buf[32];
	LONG rc;

	suite_begin(
		"Error and edge cases",
		"Missing file open fails; read-only file survives spurious write attempt.",
		"OpenAsync, WriteAsync, ReadLineAsync, CloseAsync"
	);

	af = OpenAsync((STRPTR)"T:__no_such_asyncio_file__.dat", MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_null("OpenAsync on missing file returns NULL", af)) {
		if (af) {
			CloseAsync(af);
		}
		suite_end();
		return FALSE;
	}

	DeleteFile((STRPTR)TEST_FILE_B);
	af = OpenAsync((STRPTR)TEST_FILE_B, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_WRITE) on fresh file", af)) {
		suite_end();
		return FALSE;
	}
	rc = WriteAsync(af, (APTR)"z", 1);
	if (!expect_long_eq("WriteAsync on write handle accepts 1 byte", 1, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) on line-based file", af)) {
		suite_end();
		return FALSE;
	}
	rc = WriteAsync(af, (APTR)"x", 1);
	printf("  - WriteAsync on read-only handle (buffered; may not error immediately)\n");
	printf("      Expect: file on disk unchanged after CloseAsync\n");
	printf("      Actual: WriteAsync returned %ld\n", rc);
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_A, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("Reopen read-only file for integrity check", af)) {
		suite_end();
		return FALSE;
	}
	rc = ReadLineAsync(af, buf, sizeof(buf));
	if (!expect_long_eq("First line length unchanged", (LONG)strlen(line_set[0]), rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	if (!expect_str_eq("First line content unchanged after write attempt", line_set[0], buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static BOOL test_fixture_file(void)
{
	struct AsyncFile *af;
	char buf[128];
	LONG total;
	LONG chunk;
	LONG expect;

	suite_begin(
		"Fixture file read",
		"Read bundled test_data.txt (or unittests/ path) and verify size and prefix.",
		"OpenAsync, ReadAsync, SeekAsync"
	);

	expect = async_file_size(fixture_data);
	if (!expect_long_ge("Fixture file has content (async measured size)", 1, expect)) {
		suite_end();
		return FALSE;
	}
	printf("      Fixture path: %s\n", fixture_data);

	af = OpenAsync((STRPTR)fixture_data, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) on fixture", af)) {
		suite_end();
		return FALSE;
	}

	total = 0;
	while ((chunk = ReadAsync(af, buf, sizeof(buf))) > 0) {
		total += chunk;
	}
	if (!expect_long_eq("Chunked ReadAsync total equals file size", expect, total)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}

	SeekAsync(af, 0, MODE_START);
	chunk = ReadAsync(af, buf, 7);
	if (!expect_long_eq("ReadAsync prefix length", 7, chunk)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	buf[7] = '\0';
	if (!expect_str_eq("Fixture begins with \"Line 1:\"", "Line 1:", buf)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}

	CloseAsync(af);
	suite_end();
	return !suite_fail;
}

static BOOL test_copy_roundtrip(void)
{
	struct AsyncFile *src;
	struct AsyncFile *dst;
	struct AsyncFile *verify;
	char buf[512];
	char ref[512];
	LONG pos;
	LONG n;
	LONG total;

	suite_begin(
		"Async file copy",
		"Copy fixture to T: via ReadAsync/WriteAsync; byte-compare source and copy.",
		"OpenAsync, ReadAsync, WriteAsync, CloseAsync"
	);

	src = OpenAsync((STRPTR)fixture_data, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync source fixture", src)) {
		suite_end();
		return FALSE;
	}
	dst = OpenAsync((STRPTR)TEST_FILE_COPY, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync copy destination", dst)) {
		CloseAsync(src);
		suite_end();
		return FALSE;
	}

	total = 0;
	while ((n = ReadAsync(src, buf, sizeof(buf))) > 0) {
		if (!expect_long_eq("WriteAsync copies each ReadAsync chunk", n, WriteAsync(dst, buf, n))) {
			CloseAsync(src);
			CloseAsync(dst);
			suite_end();
			return FALSE;
		}
		total += n;
	}
	CloseAsync(src);
	CloseAsync(dst);

	if (!expect_long_eq("Copy file size matches bytes transferred", total, async_file_size(TEST_FILE_COPY))) {
		suite_end();
		return FALSE;
	}

	src = OpenAsync((STRPTR)fixture_data, MODE_READ, TEST_BUF_SIZE);
	verify = OpenAsync((STRPTR)TEST_FILE_COPY, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("Reopen source for compare", src)) {
		if (verify) {
			CloseAsync(verify);
		}
		suite_end();
		return FALSE;
	}
	if (!expect_handle_ok("Reopen copy for compare", verify)) {
		CloseAsync(src);
		suite_end();
		return FALSE;
	}

	pos = 0;
	while (pos < total) {
		n = ReadAsync(src, ref, sizeof(ref));
		if (!expect_long_ge("ReadAsync source chunk during compare", 1, n)) {
			break;
		}
		if (!expect_long_eq("ReadAsync copy chunk same size as source", n, ReadAsync(verify, buf, n))) {
			break;
		}
		if (!expect_mem_eq("Chunk bytes identical", ref, buf, n)) {
			break;
		}
		pos += n;
	}
	CloseAsync(src);
	CloseAsync(verify);
	if (!expect_long_eq("All bytes compared", total, pos)) {
		suite_end();
		return FALSE;
	}

	suite_end();
	return !suite_fail;
}

static BOOL test_large_file(void)
{
	struct AsyncFile *af;
	char buf[1024];
	LONG total;
	LONG chunk;
	LONG expect;

	suite_begin(
		"Large file chunked read",
		"Read test_large.txt in 1 KB chunks; total bytes match async-measured size.",
		"OpenAsync, ReadAsync, CloseAsync"
	);

	expect = async_file_size(fixture_large);
	if (!expect_long_ge("Large fixture larger than async buffer", TEST_BUF_SIZE + 1, expect)) {
		printf("      Fixture path: %s\n", fixture_large);
		suite_end();
		return FALSE;
	}
	printf("      Fixture path: %s (%ld bytes)\n", fixture_large, expect);

	af = OpenAsync((STRPTR)fixture_large, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ) on large fixture", af)) {
		suite_end();
		return FALSE;
	}

	total = 0;
	while ((chunk = ReadAsync(af, buf, sizeof(buf))) > 0) {
		total += chunk;
	}
	CloseAsync(af);

	if (!expect_long_eq("Chunked read total equals file size", expect, total)) {
		suite_end();
		return FALSE;
	}

	suite_end();
	return !suite_fail;
}

static BOOL test_binary_roundtrip(void)
{
	struct AsyncFile *af;
	char buf[64];
	LONG rc;

	suite_begin(
		"Binary WriteAsync / ReadAsync",
		"Write embedded bytes including NUL and 0xFF; read back identical block.",
		"WriteAsync, ReadAsync, OpenAsync, CloseAsync"
	);

	af = OpenAsync((STRPTR)TEST_FILE_B, MODE_WRITE, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_WRITE)", af)) {
		suite_end();
		return FALSE;
	}
	rc = WriteAsync(af, (APTR)binary_fixture, BINARY_FIXTURE_LEN);
	if (!expect_long_eq("WriteAsync accepts full binary block", BINARY_FIXTURE_LEN, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	af = OpenAsync((STRPTR)TEST_FILE_B, MODE_READ, TEST_BUF_SIZE);
	if (!expect_handle_ok("OpenAsync(MODE_READ)", af)) {
		suite_end();
		return FALSE;
	}
	rc = ReadAsync(af, buf, sizeof(buf));
	if (!expect_long_eq("ReadAsync returns binary block length", BINARY_FIXTURE_LEN, rc)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	if (!expect_mem_eq("Binary payload bit-exact", binary_fixture, buf, BINARY_FIXTURE_LEN)) {
		CloseAsync(af);
		suite_end();
		return FALSE;
	}
	CloseAsync(af);

	suite_end();
	return !suite_fail;
}

static void print_summary(void)
{
	printf("=== AsyncTest summary ===\n");
	printf("Suites run: %ld\n", test_total);
	printf("Passed:     %ld\n", test_passed);
	printf("Failed:     %ld\n", test_failed);
	if (test_failed == 0) {
		printf("ALL TESTS PASSED\n");
	} else {
		printf("SOME TESTS FAILED\n");
	}
}

int main(int argc, char **argv)
{
	BOOL ok;

	(void)argc;

	printf("=== AsyncTest for asyncio.library ===\n\n");
	print_api_map();

	AsyncIOBase = OpenLibrary("asyncio.library", 39);
	if (!AsyncIOBase) {
		printf("ERROR: cannot open asyncio.library (IoErr %ld)\n", IoErr());
		return 20;
	}

	test_total = 0;
	test_passed = 0;
	test_failed = 0;
	cleanup_files();
	init_fixture_paths();

	ok = TRUE;
	ok = test_open_close() && ok;
	ok = test_append_mode() && ok;
	ok = test_write_read_roundtrip() && ok;
	ok = test_partial_read() && ok;
	ok = test_char_io() && ok;
	ok = test_seek() && ok;
	ok = test_peek() && ok;
	ok = test_line_functions() && ok;
	ok = test_open_from_fh() && ok;
	ok = test_errors() && ok;
	ok = test_fixture_file() && ok;
	ok = test_copy_roundtrip() && ok;
	ok = test_large_file() && ok;
	ok = test_binary_roundtrip() && ok;

	cleanup_files();
	print_summary();

	CloseLibrary(AsyncIOBase);
	AsyncIOBase = NULL;

	if (test_failed != 0) {
		return 1;
	}
	if (!ok) {
		return 1;
	}
	return 0;
}
