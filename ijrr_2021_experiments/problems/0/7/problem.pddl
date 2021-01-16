(define (problem ijrr_7)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street30 street42 - street
		building7 building16 building25 building35 - building
	)
	(:init
		(atLocation survivor0 street42)
		(atLocation survivor1 building35)
		(onFire building7)
		(onFire building16)
		(underRubble street30)
		(underRubble building25)
		(needsRepair building7)
		(needsRepair building16)
		(needsRepair building25)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street30))
		(not (needsRepair building7))
		(not (needsRepair building16))
		(not (needsRepair building25))
	))
	(:metric minimize total-time)
)
